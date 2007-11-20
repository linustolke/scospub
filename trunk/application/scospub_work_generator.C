// SCOSPUB - tools supporting Open Source development.
// Copyright (C) 2007 Linus Tolke
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option)
// any later version.
//
// Based on the sample_work_generator.C belonging to
// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


// scospub_work_generator.C: creates work for an os project
// Initially this will be hardcoded but eventually this is
// controlled by settings in the database.
//
// This work generator has the following properties
// (you may need to change some or all of these):
//
// - Runs as a daemon, and creates work when new commits are available
// - Creates work for the open source project applications
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so that they're unique.

// TODO: OOD!

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define REPLICATION_FACTOR 1
#define MAX_LINE_LENGTH 500

// globals
//
char* wu_template;
DB_APP app;
int start_time;
int seqno;
SCHED_CONFIG config;

// create one new job
//
// TODO: Hardcoded svn, project
int make_job(int rev) {
    DB_WORKUNIT wu;
    char name[256], path[256];
    const char* infiles[1];
    int retval;

    // make a unique name (for the job and its input file)
    //
    // TODO: Hardcoded Project and tool
    sprintf(name, "acpp-checkstyle_%d_%d", start_time, seqno++);

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    FILE* f = fopen(path, "w");
    if (!f) return ERR_FOPEN;

    // TODO: The input file contents, it should control the build.
    fprintf(f, "This is the input file for job %s\n", name);
    if (rev > 0)
    {
	fprintf(f, "Do for revision %d\n", rev);
    }

    fclose(f);

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
    wu.rsc_fpops_est = 1e12;
    wu.rsc_fpops_bound = 1e14;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 86400;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
    infiles[0] = name;

    // Register the job with BOINC
    //
    return create_work(
        wu,
        wu_template,
	// TODO: Hardcoded project and tool
        "templates/checkstyle_result",
        "../templates/checkstyle_result",
        infiles,
        1,
        config
    );
}

void main_loop()
{
    int retval;

    while (1) // perpetually
    {
	bool projects = true;
	while (projects)
	{
	    projects = false;
	    check_stop_daemons();

	    char tempfile[100];

	    strcpy(tempfile, "/tmp/scospubWG.XXXXXXXX");

	    char * filename = mktemp(tempfile);
	    // TODO: errorcheck!
	    std::string commandline;

	    // TODO: Hardcoded project
	    char * svn_url =
		"http://argouml-cpp.tigris.org/svn/argouml-cpp/trunk/src";
	    // TODO: Hardcoded project
	    char * svn_username = "guest";
	    // TODO: Hardcoded project
	    char * svn_password = "";

	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Fetching svn data from %s\n", svn_url
		    );

	    // TODO: Hardcoded svn
	    commandline.append("svn info ");
	    commandline.append(svn_url);
	    commandline.append(" --username '");
	    commandline.append(svn_username);
	    commandline.append("' --password '");
	    commandline.append(svn_password);
	    commandline.append("' > '");
	    commandline.append(filename);
	    commandline.append("' 2>&1");

	    int result = system(commandline.c_str());

	    if (result != 0)
	    {
		log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
				    "Cannot fetch data from %s. "
				    "Error %d\n",
				    svn_url,
				    result
			);

		// TODO: Hardcoded to allow testing without repository
		// access.
		make_job(0);
		sleep(1000);

		// TODO: Hardcoded
		sleep(100);

		unlink(filename);
		continue;
	    }

	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Fetching svn data from %s done\n",
				svn_url
		    );


	    // Parse to get the Last Changed Rev.
	    std::string line;

	    std::ifstream file(filename, std::ios::in);
	    // TODO: Error check

	    // TODO: Hardcoded svn
	    int foundrev = 0;
	    while (getline(file, line))
	    {
#define SVN_LCR "Last Changed Rev: "
		if (line.compare(0,
				 strlen("Last Changed Rev: "),
				 SVN_LCR)
		    == 0)
		{
		    std::string sub(line.substr(strlen(SVN_LCR)));
		    foundrev = atoi(sub.c_str());
		    if (foundrev > 0)
		    {
			break;
		    }
		}
	    }
	    file.close();
	    unlink(filename);

	    if (foundrev == 0)
	    {
		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "No revision found.\n"
			);
		continue;
	    }

	    // TODO: 
	    static int lastrev = 0;
	    if (foundrev <= lastrev)
	    {
		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "Revision %d is not newer than %d.\n",
				    foundrev,
				    lastrev
			);
		continue;
	    }

	    // TODO: Hardcoded project
	    lastrev = foundrev;

	    // TODO: Hardcoded project
	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Creating job for revision %d.\n",
				lastrev
		    );
	    make_job(lastrev);
	}

	// TODO: Hardcoded project
	// Take it easy!
	sleep(60);
    }
}

int main(int argc, char** argv) {
    int i, retval;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "bad cmdline arg: %s", argv[i]
            );
        }
    }

    if (config.parse_file("..")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "can't read config file\n"
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't open db\n");
        exit(1);
    }
    // TODO: Hardcoded!
    if (app.lookup("where name='acpp'")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't find app\n");
        exit(1);
    }
    // TODO: Hardcoded!
    if (read_file_malloc("../templates/checkstyle_wu", wu_template)) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't read WU template\n");
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    main_loop();
}
