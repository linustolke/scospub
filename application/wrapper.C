// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// wrapper.C
// wrapper program - lets you use non-BOINC apps with BOINC
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
//
// Does NOT handle:
// - checkpointing
// If your app does checkpointing,
// and there's some way to figure out when it's done it,
// this program could be modified to report to the core client.
//
// See http://boinc.berkeley.edu/wrapper.php for details
// Contributor: Andrew J. Younge (ajy4490@umiacs.umd.edu)

#include <stdio.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#include "procinfo.h"
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"

#define JOB_FILENAME "job.xml"
#define CHECKPOINT_FILENAME "checkpoint.txt"

using std::vector;
using std::string;

struct TASK {
    string application;
    string stdin_filename;
    string stdout_filename;
    string stderr_filename;
    string command_line;
    double final_cpu_time;
    double starting_cpu;
        // how much CPU time was used by tasks before this in the job file
#ifdef _WIN32
    HANDLE pid_handle;
    HANDLE thread_handle;
#else
    int pid;
#endif
    int parse(XML_PARSER&);
    bool poll(int& status);
    int run(int argc, char** argv);
    void kill();
    void stop();
    void resume();
	double cpu_time();
};

vector<TASK> tasks;

bool app_suspended = false;

int TASK::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    final_cpu_time = 0;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "SCHED_CONFIG::parse(): unexpected text %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/task")) {
            return 0;
        }
        else if (xp.parse_string(tag, "application", application)) continue;
        else if (xp.parse_string(tag, "stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string(tag, "stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string(tag, "stderr_filename", stderr_filename)) continue;
        else if (xp.parse_string(tag, "command_line", command_line)) continue;
    }
    return ERR_XML_PARSE;
}

int parse_job_file() {
    MIOFILE mf;
    char tag[1024], buf[256];
    bool is_tag;

    boinc_resolve_filename(JOB_FILENAME, buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        fprintf(stderr, "can't open job file %s\n", buf);
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);


    if (!xp.parse_start("job_desc")) return ERR_XML_PARSE;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "SCHED_CONFIG::parse(): unexpected text %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/job_desc")) {
            return 0;
        }
        if (!strcmp(tag, "task")) {
            TASK task;
            int retval = task.parse(xp);
            if (!retval) {
                tasks.push_back(task);
            }
        }
    }
    return ERR_XML_PARSE;
}

#ifdef _WIN32
// CreateProcess() takes HANDLEs for the stdin/stdout.
// We need to use CreateFile() to get them.  Ugh.
//
HANDLE win_fopen(const char* path, const char* mode) {
	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	if (!strcmp(mode, "r")) {
		return CreateFile(
			path,
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_EXISTING,
			0, 0
		);
	} else if (!strcmp(mode, "w")) {
		return CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
	} else if (!strcmp(mode, "a")) {
		HANDLE hAppend = CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
        SetFilePointer(hAppend, 0, NULL, FILE_END);
        return hAppend;
	} else {
		return 0;
	}
}
#endif

// the "state file" might tell us which app we're in the middle of,
// what the starting CPU time is, etc.
// Not implemented yet.
//
void parse_state_file() {
}

int TASK::run(int argct, char** argvt) {
    string app_path, stdout_path, stdin_path, stderr_path;

    boinc_resolve_filename_s(application.c_str(), app_path);

    // Append wrapper's command-line arguments to those in the job file.
    //
    for (int i=1; i<argct; i++){
        command_line += argvt[i];
        if ((i+1) < argct){
            command_line += string(" ");
        }
    }   

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    string command;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    command = app_path + string(" ") + command_line;

    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
	if (stdout_filename != "") {
		boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
		startup_info.hStdOutput = win_fopen(stdout_path.c_str(), "w");
	}
	if (stdin_filename != "") {
		boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
		startup_info.hStdInput = win_fopen(stdin_path.c_str(), "r");
	}
    if (stderr_filename != "") {
        boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
        startup_info.hStdError = win_fopen(stderr_path.c_str(), "w");
    } else {
        startup_info.hStdError = win_fopen(STDERR_FILE, "a");
    }
             
    if (!CreateProcess(
        app_path.c_str(),
        (LPSTR)command.c_str(),
        NULL,
        NULL,
        TRUE,		// bInheritHandles
        CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
        NULL,
        NULL,
        &startup_info,
        &process_info
    )) {
        return ERR_EXEC;
    }
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
    SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
#else
    int retval, argc;
    char progname[256], buf[256];
    char* argv[256];
    char arglist[4096];
	FILE* stdout_file;
	FILE* stdin_file;
	FILE* stderr_file;

    pid = fork();
    if (pid == -1) {
        boinc_finish(ERR_FORK);
    }
    if (pid == 0) {
		// we're in the child process here
		//
		// open stdout, stdin if file names are given
		// NOTE: if the application is restartable,
		// we should deal with atomicity somehow
		//
		if (stdout_filename != "") {
			boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
			stdout_file = freopen(stdout_path.c_str(), "w", stdout);
			if (!stdout_file) return ERR_FOPEN;
		}
		if (stdin_filename != "") {
			boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
			stdin_file = freopen(stdin_path.c_str(), "r", stdin);
			if (!stdin_file) return ERR_FOPEN;
		}
        if (stderr_filename != "") {
            boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
            stderr_file = freopen(stderr_path.c_str(), "w", stderr);
            if (!stderr_file) return ERR_FOPEN;
        }

		// construct argv
        // TODO: use malloc instead of stack var
        //
        strcpy(buf, app_path.c_str());
        argv[0] = buf;
        strlcpy(arglist, command_line.c_str(), sizeof(arglist));
        argc = parse_command_line(arglist, argv+1);
        fprintf(stderr, "wrapper: running %s (%s)\n", buf, arglist);
        setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY);
        retval = execv(buf, argv);
        exit(ERR_EXEC);
    }
#endif
    return 0;
}

bool TASK::poll(int& status) {
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            status = exit_code;
            final_cpu_time = cpu_time();
            return true;
        }
    }
#else
    int wpid, stat;
    struct rusage ru;

    wpid = wait4(pid, &status, WNOHANG, &ru);
    if (wpid) {
        final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
        return true;
    }
#endif
    return false;
}

void TASK::kill() {
#ifdef _WIN32
    TerminateProcess(pid_handle, -1);
#else
    ::kill(pid, SIGKILL);
#endif
}

void TASK::stop() {
#ifdef _WIN32
    SuspendThread(thread_handle);
#else
    ::kill(pid, SIGSTOP);
#endif
}

void TASK::resume() {
#ifdef _WIN32
    ResumeThread(thread_handle);
#else
    ::kill(pid, SIGCONT);
#endif
}

void poll_boinc_messages(TASK& task) {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat) {
        task.kill();
        exit(0);
    }
    if (status.quit_request) {
        task.kill();
        exit(0);
    }
    if (status.abort_request) {
        task.kill();
        exit(0);
    }
    if (status.suspended) {
        if (!app_suspended) {
            task.stop();
            app_suspended = true;
        }
    } else {
        if (app_suspended) {
            task.resume();
            app_suspended = false;
        }
    }
}

double TASK::cpu_time() {
#ifdef _WIN32
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER tKernel, tUser;
    LONGLONG totTime;

    int retval = GetProcessTimes(
        pid_handle, &creation_time, &exit_time, &kernel_time, &user_time
    );
    if (retval == 0) return 0;

    tKernel.LowPart  = kernel_time.dwLowDateTime;
    tKernel.HighPart = kernel_time.dwHighDateTime;
    tUser.LowPart    = user_time.dwLowDateTime;
    tUser.HighPart   = user_time.dwHighDateTime;
    totTime = tKernel.QuadPart + tUser.QuadPart;

    return totTime / 1.e7;
#else
    return  linux_cpu_time(pid);
#endif
}

void send_status_message(TASK& task, double frac_done) {
    boinc_report_app_status(
        task.starting_cpu + task.cpu_time(),
        task.starting_cpu,
        frac_done
    );
}

// Support for multiple tasks.
// We keep a checkpoint file that says how many tasks we've completed
// and how much CPU time has been used so far
//
void write_checkpoint(int ntasks, double cpu) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%d %f\n", ntasks, cpu);
    fclose(f);
}

void read_checkpoint(int& ntasks, double& cpu) {
    int nt;
    double c;

    ntasks = 0;
    cpu = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%d %lf", &nt, &c);
    if (n != 2) return;
    ntasks = nt;
    cpu = c;
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval;
    int ntasks;
    double cpu;

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    fprintf(stderr, "wrapper: starting\n");
    boinc_init_options(&options);
    retval = parse_job_file();
    if (retval) {
        fprintf(stderr, "can't parse job file: %d\n", retval);
        boinc_finish(retval);
    }

    parse_state_file();

    read_checkpoint(ntasks, cpu);
    if (ntasks > (int)tasks.size()) {
        fprintf(stderr, "Checkpoint file: ntasks %d too large\n", ntasks);
        boinc_finish(1);
    }
    for (unsigned int i=ntasks; i<tasks.size(); i++) {
        TASK& task = tasks[i];
        double frac_done = ((double)i)/((double)tasks.size());

        fprintf(stderr, "running %s\n", task.application.c_str());
        task.starting_cpu = cpu;
        retval = task.run(argc, argv);
        if (retval) {
            fprintf(stderr, "can't run app: %d\n", retval);
            boinc_finish(retval);
        }
        while(1) {
            int status;
            if (task.poll(status)) {
                if (status) {
                    fprintf(stderr, "app error: 0x%x\n", status);
                    boinc_finish(status);
                }
                break;
            }
            poll_boinc_messages(task);
            send_status_message(task, frac_done);
            boinc_sleep(1.);
        }
        cpu += task.final_cpu_time;
        write_checkpoint(i+1, cpu);
    }
    boinc_finish(0);
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif
