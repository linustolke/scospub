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
//   and revision, so that they're unique.
//
// There could be another tool generating jobs for all old revisions.

// TODO: OOD!

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#include <map>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#include "scospub_db.h"

#define REPLICATION_FACTOR 1
#define MAX_LINE_LENGTH 500

// globals
//
char* wu_template = NULL;
DB_APP app;
int start_time;
SCHED_CONFIG config;

typedef std::map<const int, int> revisions_map_type;


// create one new job
//
int make_job(const SCOS_PROJECT project, const SCOS_TOOL tool,
	     revisions_map_type svn_revisions)
{
    // Files to create:
    // job.xml different for each job
    // input files (any?)

    static int seqno = 1;
    int urlid = 1;

    // make a unique name (for the job and its input file)
    //
    // This name is also used by the work assimilation to enter
    // the result in the correct way.
    // TODO: Hardcoded Project, toolid, tool.
    char name[255];
    sprintf(name,
	    "%s_%s-%d-%d_%d",
	    tool.name,
	    project.name,
	    seqno++,
	    tool.id,
	    start_time);

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    char path[255];
    int retval = config.download_path(name, path);
    if (retval) return retval;

    std::ofstream f(path);
    if (!f.is_open())
    {
        return ERR_FOPEN;
    }

    // TODO: Hardcoded
    f << "<job_desc>\n";
    f << "  <project>" << project.name << "</project>\n";
    f << "  <tool>" << tool.name << "</tool>\n";
    f << "  <toolid>" << tool.id << "</toolid>\n";
    f << "  <config>" << tool.config << "</config>\n";

    DB_SCOS_SOURCE source;

    // Mapping a project to url
    f << "  <source>\n";
    while (!source.enumerate()) {
	f << "    <svn id='" << source.id << "'>\n";
	f << "      <url>" << source.url << "</url>\n";
	const char * checkoutdir = source.url + strlen(source.rooturl);
	f << "      <checkoutdir>" << checkoutdir << "</checkoutdir>\n";
	f << "      <username>" << source.username << "</username>\n";
	f << "      <password>" << source.password << "</password>\n";
	f << "      <revision>" << svn_revisions[source.id] << "</revision>\n";
	f << "    </svn>\n";
    }
    f << "  </source>\n";
    source.end_enumerate();


    while (!source.enumerate()) {
	const char * checkoutdir = source.url + strlen(source.rooturl);

	f << "  <task>\n";
	f << "    <application>";
	f << "svn";
	f << "</application>\n";
	f << "    <command_line>";

	f << "co";
	f << " -r " << svn_revisions[source.id];

	// Mapping a project to url
	f << " --no-auth-cache";
	f << " " << source.url;

	// TODO: Part url! checkoutdir!
	f << " /tmp/scospub/" << project.name << "/trunk/src";
	f << " --username '" << source.username << "' ";
	f << " --password '" << source.password << "'";

	f << "</command_line>\n";
	f << "  </task>\n";
    }
    source.end_enumerate();

    f << "  <task>\n";
    f << "    <application>";
    // TODO: Mapping tool to application?
    f << "checkstyle-java";
    f << "</application>\n";
    f << "    <stdout_filename>";
    f << "out1";
    f << "</stdout_filename>\n";
    f << "    <stderr_filename>";
    f << "out2";
    f << "</stderr_filename>\n";

    f << "    <command_line>";
    // TODO: Mapping tool to application?
    f << "-cp checkstyle-all-4.3.jar";
    f << " com.puppycrawl.tools.checkstyle.Main";
    f << " -c checkstyle-sun_checks.xml";
    // TODO: Part url! checkoutdir!
    f << " -r /tmp/scospub/" << project.name;
    f << "</command_line>\n";
    f << "  </task>\n";
    f << "</job_desc>\n";

    f.close();

    // Fill in the job parameters
    //
    DB_WORKUNIT wu;
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

    const int num_infiles = 3;
    const char * infiles[num_infiles];
    infiles[0] = name;
    infiles[1] = "checkstyle-all-4.3.jar";
    infiles[2] = "checkstyle-sun_checks.xml";

    // TODO: Hardcoded!
    if (wu_template == NULL)
    {
        // TODO: Mapping tool to wu file?

        if (read_file_malloc("../templates/checkstyle_wu", wu_template))
	{
	    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't read WU template\n");
	    exit(1);
	}
    }

    // Register the job with BOINC
    //
    create_work(
        wu,
        wu_template,
	// TODO: Hardcoded project and tool
        "templates/scospub2_result",
        "../templates/scospub2_result",
        infiles,
        num_infiles,
        config
    );

    while (!source.enumerate()) {
	source.lastrevision = svn_revisions[source.id];
	source.update();
    }
    source.end_enumerate();
}

void main_loop()
{
    int retval;

    while (1) // perpetually
    {
	DB_SCOS_PROJECT project;

	while (!project.enumerate("WHERE active = TRUE"))
	{
	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Processing project %s\n", project.name
		    );

	    check_stop_daemons();

	    DB_SCOS_SOURCE source;

	    // TODO: Hardcoded to svn
	    revisions_map_type svn_revisions;

	    int changes = 0;

	    char clause[100];
	    sprintf(clause, "WHERE project = %d", project.id);

	    while (!source.enumerate(clause))
	    {
		if (source.type != 1)
		{
		    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
					"Cannot handle source of type %d.\n",
					source.type);
		    continue;
		}

		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "Polling %s for project %s\n",
				    source.url,
				    project.name
		    );

		char tempfile[100];

		strcpy(tempfile, "/tmp/scospubWG.XXXXXX");

		char * filename = mktemp(tempfile);

		// TODO: errorcheck!
		std::string commandline;

		// TODO: Hardcoded svn
		commandline.append("svn info ");
		commandline.append(source.url);
		commandline.append(" --username '");
		commandline.append(source.username);
		commandline.append("' --password '");
		commandline.append(source.password);
		commandline.append("' > '");
		commandline.append(filename);
		commandline.append("' 2>&1");

		int result = system(commandline.c_str());

		if (result != 0)
		{
		    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
					"Cannot fetch data from %s. "
					"Error %d\n",
					source.url,
					result
			);

		    unlink(filename);

		    sleep(1);
		    continue;
		}

		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "Fetching svn data from %s (for project %s) done\n",
				    source.url,
				    project.name
		    );


		// Parse to get the Last Changed Rev.
		std::string line;

		std::ifstream file(filename, std::ios::in);
		// TODO: Error check

		// TODO: Hardcoded to svn
		int foundrev = 0;
		while (getline(file, line))
		{
#define SVN_LCR "Last Changed Rev: "
		    if (line.compare(0, strlen(SVN_LCR), SVN_LCR)
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
					"No revision found for %s.\n",
					source.url
			);
		    continue;
		}

		svn_revisions[source.id] = foundrev;

		if (foundrev <= source.lastrevision)
		{
		    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
					"Revision %d is not newer than %d.\n",
					foundrev,
					source.lastrevision
			);
		    continue;
		}
		changes++;
	    }
	    source.end_enumerate();

	    if (changes > 0)
	    {
		// There are indeed changes to some of the sources.

		DB_SCOS_TOOL tool;

		while (!tool.enumerate(clause))
		{
		    // Creat one job per tool.
		    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
					"Creating job for %s %s %s.\n",
					project.name, tool.name, tool.config
			);
		    make_job(project, tool, svn_revisions);
		}
	    }

	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Processing project %s done\n", project.name
		    );
	}

	// Wait a while between every polling.
	// TODO: Tune this.
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
    if (app.lookup("where name='wrapper'")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't find app\n");
        exit(1);
    }

    start_time = time(0);

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    main_loop();
}
