// Static Checks for Open Source Projects Using Boinc
// http://scospub.googlecode.com
// Copyright (C) 2007, 2008 Linus Tolke
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

// scospubapp.C
// application wrapper program for scospub
//
// This was based on the wrapper program from the boinc distribution.
// See http://boinc.berkeley.edu/wrapper.php for details
// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// Functions from wrapper are kept to handle:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
//

#include <stdio.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "procinfo.h"
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"

#include "scospub_parse.h"
#include "scospub_xml_tags.h"

#define JOB_FILENAME "sc.xml"
#define CHECKPOINT_FILENAME "checkpoint.txt"

using std::vector;
using std::string;

double final_cpu_time = 0.0;
double checkpoint_cpu = 0.0;

class JOB_INFO {
public:
    virtual const char * get_project_name() const = 0;
};

class TASK {
public:
    virtual const char * get_application() const = 0;

    virtual int parse(sx_parser&) = 0;
    virtual bool poll(int& status) = 0;
    virtual int run(JOB_INFO * info) = 0;
    virtual void kill() = 0;
    virtual void stop() = 0;
    virtual void resume() = 0;
    virtual double get_current_cpu_time() = 0;
};

class processTASK : public TASK {
private:
    string application;
    string command_line;
    string file_args_unprocessed;

    string stdin_filename;
    string stdout_filename;
    string stderr_filename;
    bool ignore_exit;
#ifdef _WIN32
    HANDLE pid_handle;
    HANDLE thread_handle;
#else
    int pid;
#endif

protected:
    void set_application(const char * str) { application = str; }
    void set_command_line(const char * str) { command_line = str; }

    bool parse_in_out_files(sx_parser& xp, char * tag);
    bool parse_cmd_line(sx_parser& xp, char * tag);

    virtual vector<string> get_processed_args(JOB_INFO * info);

public:
    // From TASK
    virtual const char * get_application() const;
    virtual int parse(sx_parser&);
    virtual int run(JOB_INFO * info);
    virtual bool poll(int& status);
    virtual void kill();
    virtual void stop();
    virtual void resume();
    virtual double get_current_cpu_time();
};


class javaTASK : public processTASK
{
public:
    // From TASK
    virtual int parse(sx_parser&);
    virtual int run(JOB_INFO * info);
};


class source : public processTASK
{
private:
    string id;
};

class svn_source : public source
{
private:
    int id;
    string url;
    string checkoutdir;
    string username;
    string password;
    int revision;

public:
    svn_source(int i) : id(i) {};
    virtual int parse(sx_parser&);
    virtual int run(JOB_INFO * info);
};

class sources
{
private:
    vector<source*> sources;

public:
    void push_back_all(vector<TASK*>& tasks) const;

    virtual int parse(sx_parser&);
};


class job_type : JOB_INFO
{
private:
    string project_name;
    int projectid;
    string tool;
    int toolid;
    string config;
    
    int ntasks;
    double cpu;

    vector<TASK*> tasks;

public:
    virtual const char * get_project_name() const {
	return project_name.c_str();
    };

    int parse();

    void run();
};


bool app_suspended = false;


// Parse a svn source entry
int svn_source::parse(sx_parser& xp)
{
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag))
    {
        if (!is_tag)
	{
            fprintf(stderr,
		    "SCHED_CONFIG::parse(): unexpected text %s in "
		    TAG_SVN "\n",
		    tag);
            continue;
        }

	if (!strcmp(tag, "/" TAG_SVN))
	{
	    return 0;
	}
	else if (xp.parse_string(tag, TAG_URL, url))
	    continue;
	else if (xp.parse_string(tag, TAG_CHECKOUTDIR, checkoutdir))
	    continue;
	else if (xp.parse_string(tag, TAG_USERNAME, username))
	    continue;
	else if (xp.parse_string(tag, TAG_PASSWORD, password))
	    continue;
	else if (xp.parse_int(tag, TAG_REVISION, revision))
	    continue;

	fprintf(stderr,
		"SCHED_CONFIG::parse(): unexpected tag %s in " TAG_SVN ".\n",
		tag);
    }
}


int svn_source::run(JOB_INFO * info)
{
    set_application("/usr/bin/svn"); // TODO: Different for different clients

    string command_line = "co";
    command_line += " -r ";

    char revarea[15];
    sprintf(revarea, "%d", revision);
    command_line += revarea;

    command_line += " --no-auth-cache ";
    command_line += url;

    command_line += " /tmp/scospub/";
    command_line += info->get_project_name();
    command_line += "/";
    command_line += checkoutdir;

    command_line += " --username '";
    command_line += username;
    command_line += "' --password '";
    command_line += password;
    command_line += "'";

    set_command_line(command_line.c_str());

    return processTASK::run(info);
}


// Parse the source
int sources::parse(sx_parser& xp)
{
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag))
    {
        if (!is_tag)
	{
            fprintf(stderr,
		    "SCHED_CONFIG::parse(): unexpected text %s in "
		    TAG_SOURCE "\n",
		    tag);
            continue;
        }

	int id;

        if (!strcmp(tag, "/" TAG_SOURCE))
	{
            return 0;
        }
	else if (xp.match_tag(tag, TAG_SVN, id))
	{
	    svn_source * source = new svn_source(id);
	    int retval = source->parse(xp);
	    if (!retval) {
		sources.push_back(source);
	    }
	    else
	    {
		delete(source);
		break;
	    }
	    continue;
	}
	// else if (!strcmp(tag, TAG_CVS)) ...

	fprintf(stderr,
		"SCHED_CONFIG::parse(): unexpected tag %s in " TAG_SOURCE ".\n",
		tag);
		
    }
    return ERR_XML_PARSE;
}


const char * processTASK::get_application() const
{
    return application.c_str();
}

bool processTASK::parse_in_out_files(sx_parser& xp, char * tag)
{
    if (xp.parse_string(tag, TAG_STDIN_FILENAME, stdin_filename))
	return true;
    if (xp.parse_string(tag, TAG_STDOUT_FILENAME, stdout_filename))
	return true;
    if (xp.parse_string(tag, TAG_STDERR_FILENAME, stderr_filename))
	return true;
    return false;
}

bool processTASK::parse_cmd_line(sx_parser& xp, char * tag)
{
    if (xp.parse_string(tag, TAG_COMMAND_LINE, command_line))
	return true;

    int ie = 0;
    if (xp.parse_int(tag, TAG_IGNORE_EXIT, ie))
    {
	ignore_exit = false;
	if (ie != 0)
	{
	    ignore_exit = true;
	}
	return true;
    }

    if (xp.parse_string(tag, TAG_ARGS, file_args_unprocessed))
	return true;

    return false;
}


// Parse a task!
int processTASK::parse(sx_parser& xp)
{
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag))
    {
        if (!is_tag)
	{
            fprintf(stderr,
		    "processTASK::parse(): unexpected text %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/" TAG_TASK))
	{
            return 0;
        }
        else if (xp.parse_string(tag, TAG_APPLICATION, application))
	    continue;
	else if (parse_in_out_files(xp, tag))
	    continue;
	else if (parse_cmd_line(xp, tag))
	    continue;

	fprintf(stderr,
		"processTASK::parse(): unexpected tag %s\n", tag);
    }
    return ERR_XML_PARSE;
}


// Parse a java task!
int javaTASK::parse(sx_parser& xp)
{
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag))
    {
        if (!is_tag)
	{
            fprintf(stderr,
		    "javaTASK::parse(): unexpected text %s\n", tag);
            continue;
        }

        if (!strcmp(tag, "/" TAG_TASK_JAVA))
	{
            return 0;
        }
	else if (parse_in_out_files(xp, tag))
	    continue;
	else if (parse_cmd_line(xp, tag))
	    continue;

	fprintf(stderr,
		"javaTASK::parse(): unexpected tag %s\n", tag);
    }
    return ERR_XML_PARSE;
}

int javaTASK::run(JOB_INFO * info)
{
    // TODO: Different depending on the host type.
    set_application("/usr/bin/java");

    return processTASK::run(info);
}


void sources::push_back_all(vector<TASK*>& tasks) const {
    for (vector<source*>::const_iterator iter = sources.begin(); 
	 iter != sources.end();
	 iter++)
    {
	tasks.push_back(*iter);
    }
}




// Parse a file
// If <task> is found, it transfers control to processTASK->parse()
int job_type::parse() {
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
    sx_parser xp(&mf);

    if (!xp.parse_start(JOB_TAG))
	return ERR_XML_PARSE;

    while (!xp.get(tag, sizeof(tag), is_tag))
    {
        if (!is_tag)
	{
            fprintf(stderr,
		    "SCHED_CONFIG::parse(): unexpected text %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/" JOB_TAG))
	{
            return 0;
        }

        if (strcmp(tag, TAG_TASK) == 0
	    || strcmp(tag, TAG_TASK_JAVA) == 0)
	{
	    TASK * task = NULL;

            if (strcmp(tag, TAG_TASK) == 0)
	    {
		task = new processTASK();
	    }

            if (strcmp(tag, TAG_TASK_JAVA) == 0)
	    {
		task = new javaTASK();
	    }

	    if (task == NULL) {
		// Shouldn't happen.
		fprintf(stderr,
			"SCHED_CONFIG::parse(): unexpected tag %s\n", tag);
		break;
	    }

            int retval = task->parse(xp);

            if (!retval) {
                tasks.push_back(task);
            }
	    else
	    {
		delete(task);
		break;
	    }

	    continue;
        }
	else if (xp.parse_string(tag, TAG_PROJECT, project_name, projectid))
	    continue;
	else if (xp.parse_string(tag, TAG_TOOL, tool, toolid))
	    continue;
	else if (xp.parse_string(tag, TAG_CONFIG, config))
	    continue;
	else if (!strcmp(tag, TAG_SOURCE))
	{
	    sources src;
	    int retval = src.parse(xp);

            if (!retval) {
                src.push_back_all(tasks);
            }
	    else
	    {
		break;
	    }

	    continue;
	}
	// TODO: Maybe more xml entries.

	fprintf(stderr,
		"SCHED_CONFIG::parse(): unexpected tag %s in " JOB_TAG ".\n",
		tag);
    }
    return ERR_XML_PARSE;
}


vector<string>
processTASK::get_processed_args(JOB_INFO * info)
{
    // Parse through the repository and find files matching
    // file_args_unprocessed and build strings reflecting their path
    vector<string> res;

    if (file_args_unprocessed == "")
    {
	return res;
    }

#ifdef _WIN32
    choke me!
#else
    string command =
	string("find /tmp/scospub/") + info->get_project_name()
	+ string(" -name ") + file_args_unprocessed
	+ string(" -print");

    FILE * names = popen(command.c_str(), "r");

    if (names == NULL)
    {
	// Some error occured.
	fprintf(stderr, "Could not run find, error is %s.\n",
		strerror(errno));
	return res;
    }

#define LONGEST_FILENAME 1024
    char buf[LONGEST_FILENAME];
    char * readfile;

    while ((readfile = fgets(buf, LONGEST_FILENAME, names)) != NULL)
    {
	res.insert(res.end(), string(readfile));
    }

    pclose(names);
#endif

    return res;
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

#else

// Create a copy of the string on the heap.
// These strings are never freed. Instead we do execv (or exit).
static char *
create_malloced_string(const char * str)
{
    int len = strlen(str) + 1;
    char * mallstr = (char *) malloc(len);
    strlcpy(mallstr, str, len);
    return mallstr;
}
#endif

int processTASK::run(JOB_INFO * info)
{
    if (application == "")
    {
	fprintf(stderr, "The application is not set.\n");
	return ERR_EXEC;
    }

    if (command_line == "")
    {
	fprintf(stderr, "The command line is not set.\n");
	return ERR_EXEC;
    }

    string app_path;

    boinc_resolve_filename_s(application.c_str(), app_path);

    // Initialize the data structures
#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
#else
    pid = fork();
    if (pid == -1) {
        boinc_finish(ERR_FORK);
    }
    if (pid != 0) {
	return 0;
    }

    // we're in the child process now
#endif

    vector<string> processed_args = get_processed_args(info);
    
    // Initialize the command
#ifdef _WIN32
    string command;
    command = app_path + string(" ") + command_line;
    for (vector<string>::iterator iter = processed_args.begin();
	 iter != processed_args.end();
	 iter++)
    {
	command += string(" ") + *iter;
    }
#else
    char * buf = create_malloced_string(app_path.c_str());
    fprintf(stderr,
	    "scospubapp: running %s %s\n", buf, command_line.c_str());
    fflush(stderr);

    // construct argv
    char* argv[256];

    argv[0] = buf;
    char * arglist = create_malloced_string(command_line.c_str());

    int argc = parse_command_line(arglist, argv+1);
    for (vector<string>::iterator iter = processed_args.begin();
	 iter != processed_args.end();
	 iter++)
    {
	argv[argc++] = create_malloced_string((*iter).c_str());
    }
#endif

    // Fix stdin, stdout, and stderr.
#ifdef _WIN32
    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
#else
    //
    // open stdout, stdin if file names are given
    // NOTE: if the application is restartable,
    // we should deal with atomicity somehow
    //
    // The logging above is done before redirecting stderr
    // to end up on the scospubapp stderr.
    //
    FILE* stdout_file;
    FILE* stdin_file;
    FILE* stderr_file;

#endif
    if (stdout_filename != "") {
	string stdout_path;

	boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
#ifdef _WIN32
	startup_info.hStdOutput = win_fopen(stdout_path.c_str(), "w");
#else
	stdout_file = freopen(stdout_path.c_str(), "w", stdout);
	if (!stdout_file) return ERR_FOPEN;
#endif
    }
    if (stdin_filename != "") {
	string stdin_path;

	boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
#ifdef _WIN32
	startup_info.hStdInput = win_fopen(stdin_path.c_str(), "r");
#else
	stdin_file = freopen(stdin_path.c_str(), "r", stdin);
	if (!stdin_file) return ERR_FOPEN;
#endif
    }
    if (stderr_filename != "") {
	string stderr_path;

        boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
#ifdef _WIN32
        startup_info.hStdError = win_fopen(stderr_path.c_str(), "w");
    } else {
        startup_info.hStdError = win_fopen(STDERR_FILE, "a");
#else
	stderr_file = freopen(stderr_path.c_str(), "w", stderr);
	if (!stderr_file) return ERR_FOPEN;
#endif
    }
             
    // Create the process
#ifdef _WIN32
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
    setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY);
    int retval = execv(buf, argv);
    exit(ERR_EXEC);
#endif
}


// poll()
// Returns true if the process has exited, otherwise false.
// status == 0 means proceed.
// status != 0 means serious error in the application.
bool processTASK::poll(int& status) {
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
	    fprintf(stderr, "app exited: 0x%x\n", status);
            status = exit_code;
            final_cpu_time = get_current_cpu_time();

	    if (ignore_exit)
	    {
		status = 0;
	    }

            return true;
        }
    }
#else
    int wpid, stat;
    struct rusage ru;

    wpid = wait4(pid, &status, WNOHANG, &ru);
    if (wpid) {
        final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
	if (WIFEXITED(status)) {
	    status = WEXITSTATUS(status);
	    fprintf(stderr, "app exited with exit status: %d", status);


	} else if (WIFSIGNALED(status)) {
	    int sig = WTERMSIG(status);

	    fprintf(stderr, "app killed by signal: %d", sig);
#ifdef WCOREDUMP
	    if (WCOREDUMP(status)) {
		fprintf(stderr, " (core dumped)");
	    }
#endif
	    status = 1;
	} else {
	    fprintf(stderr, "app exited with unknown status 0x%x", status);
	}

	if (ignore_exit)
	{
	    fprintf(stderr, " (ignored)");
	    status = 0;
	}
	fprintf(stderr, "\n");

        return true;
    }
#endif
    return false;
}

void processTASK::kill() {
#ifdef _WIN32
    TerminateProcess(pid_handle, -1);
#else
    ::kill(pid, SIGKILL);
#endif
}

void processTASK::stop() {
#ifdef _WIN32
    SuspendThread(thread_handle);
#else
    ::kill(pid, SIGSTOP);
#endif
}

void processTASK::resume() {
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

double processTASK::get_current_cpu_time() {
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
    return linux_cpu_time(pid);
#endif
}

void send_status_message(TASK& task, double frac_done) {
    boinc_report_app_status(
        checkpoint_cpu + task.get_current_cpu_time(),
        checkpoint_cpu,
        frac_done
    );
}

// Support for multiple tasks.
// We keep a checkpoint file that says how many tasks we've completed
// and how much CPU time has been used so far.
// For failing tasks we also keep track of how many times they have failed.
//
void write_checkpoint(int ntasks, double cpu, int attempts) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%d %f %d\n", ntasks, cpu);
    fclose(f);
}

void read_checkpoint(int& ntasks, double& cpu, int& attempts) {
    int nt;
    double c;
    int a;

    ntasks = 0;
    cpu = 0;
    attempts = 0;

    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%d %lf %d", &nt, &c, &a);
    if (n != 3) return;

    ntasks = nt;
    cpu = c;
    attempts = a;
    
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    job_type job;

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    fprintf(stderr, "scospubapp: starting\n");
    boinc_init_options(&options);

    int retval = job.parse();
    if (retval) {
        fprintf(stderr, "can't parse job file: %d\n", retval);
        boinc_finish(retval);
    }

    job.run();
}

void job_type::run()
{
    int attempts;

    read_checkpoint(ntasks, cpu, attempts);
    if (ntasks > (int)tasks.size()) {
        fprintf(stderr, "Checkpoint file: ntasks %d too large\n", ntasks);
        boinc_finish(1);
    }
    for (unsigned int i = ntasks; i < tasks.size(); i++) {
        TASK& task = *tasks[i];
        double frac_done = ((double)i) / ((double)tasks.size());

        fprintf(stderr, "task %u starting\n", i);
        checkpoint_cpu = cpu;
        int retval = task.run(this);
        if (retval) {
            fprintf(stderr, "can't run app: %d\n", retval);
            boinc_finish(retval);
        }
        fprintf(stderr, "task %u started\n", i);
        while(1) {
            int status;
            if (task.poll(status)) {
                if (status) {
		    fprintf(stderr,
			    "task %u (%s) exited with exit status %d\n",
			    i,
			    task.get_application(),
			    status);
		    
		    if (attempts < 15) {
			cpu += final_cpu_time;
			write_checkpoint(i+1, cpu, attempts + 1);

			boinc_sleep(1. + 2. * attempts * attempts);
			boinc_finish(status);
		    } else {
			fprintf(stderr,
				"too many attempts, giving up on task %d.\n",
				i);
			boinc_finish(0);
		    }
                }
                break;
            }
            poll_boinc_messages(task);
            send_status_message(task, frac_done);
            boinc_sleep(1.);
        }
        cpu += final_cpu_time;
        write_checkpoint(i+1, cpu, 0);
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
