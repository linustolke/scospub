// SCOSPUB - tools supporting Open Source development.
// Copyright (C) 2007, 2008 Linus Tolke
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
#include <math.h>
#include <time.h>
#include <string>
#include <fstream>

#include <map>
#include <list>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#include "scospub_db.h"
#include "scospub_xml_tags.h"
#include "sp_log_time_cond.h"

#define REPLICATION_FACTOR 1
#define MAX_LINE_LENGTH 500

using std::string;
using std::list;

// globals
//
DB_APP app;
int start_time;

typedef std::map<const int, int> revisions_map_type;


// Interface to keep tool info together.
class TOOLDETAILS {
public:
    virtual void write_command_line(std::ofstream& f) const = 0;
    virtual void
    write_file_args(std::ofstream& f, vector<string> reldirs) const = 0;

    virtual const list<string>* get_infiles() const = 0;

    // The wu template
    virtual const char * get_wu_template() = 0;

    // The filename of the result template
    virtual const char * get_result_template_filename() = 0;


    // Static members and methods to maintain the register of the tools:
private:
    static std::map<string, TOOLDETAILS*> arch;
    static string calculate_index(const SCOS_TOOL * toolp);
public:
    static void put(const SCOS_TOOL *, TOOLDETAILS *);
    static TOOLDETAILS* lookup(const SCOS_TOOL *);
};
std::map<string, TOOLDETAILS*> TOOLDETAILS::arch;

string TOOLDETAILS::calculate_index(const SCOS_TOOL * toolp) {
    string id = "";
    id += toolp->name;
    id += "/";
    id += toolp->config;

    return id;
}

void TOOLDETAILS::put(const SCOS_TOOL * toolp, TOOLDETAILS * tdetails) {
    arch[calculate_index(toolp)] = tdetails;
}

TOOLDETAILS* TOOLDETAILS::lookup(const SCOS_TOOL * toolp) {
    return arch[calculate_index(toolp)];
}


// Abstract base class
class ABSTRACT_TOOLDETAILS : TOOLDETAILS {
private:
    list<string> infiles;
    char * wu_template;

protected:
    void add_file(string str) {
	infiles.push_back(str);
    }

    void put(const char * toolname, const char * config) {
	SCOS_TOOL tool;
	strcpy(tool.name, toolname);
	strcpy(tool.config, config);
	TOOLDETAILS::put(&tool, this);
    }

public:
    ABSTRACT_TOOLDETAILS(const char * wu_path) {
	if (read_file_malloc(wu_path, wu_template)) {
	    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't read WU template for checkstyle\n");
	    exit(1);
	}
    }

    virtual const list<string>* get_infiles() const {
	return &infiles;
    }

    virtual const char * get_wu_template() {
	return wu_template;
    }

    virtual const char * get_result_template_filename() {
	return "templates/scospub2_result";
    }

    virtual void
    write_file_args(std::ofstream& f, vector<string> reldirs) const {
    }
};

// Checkstyle
class TOOLD_CHECKSTYLE_SUNCHECKS : ABSTRACT_TOOLDETAILS {
public:
    TOOLD_CHECKSTYLE_SUNCHECKS()
	: ABSTRACT_TOOLDETAILS("../templates/checkstyle_wu") {
	add_file("checkstyle-all-4.3.jar");
	add_file("checkstyle-sun_checks.xml");

	put("checkstyle", "sunchecks");
    }

    virtual void write_command_line(std::ofstream& f) const {
	f << "-cp checkstyle-all-4.3.jar";
	f << " com.puppycrawl.tools.checkstyle.Main";
	f << " -c checkstyle-sun_checks.xml";
    }

    virtual void
    write_file_args(std::ofstream& f, vector<string> reldirs) const {
	for (vector<string>::iterator i = reldirs.begin();
	     i != reldirs.end();
	     i++) {
	    if (i != reldirs.begin()) {
		f << " ";
	    }
	    f << "-r %r/" << *i;
	}
    }
} td_cssc;

class TOOLD_FINDBUGS : ABSTRACT_TOOLDETAILS {
private:
    const char * jar;

protected:
    void add_jar(const char * file) {
	jar = file;
	add_file(jar);
    }

public:
    TOOLD_FINDBUGS(const char * version)
	: ABSTRACT_TOOLDETAILS("../templates/findbugs_wu") {
	put("findbugs", version);
    }

    virtual void write_command_line(std::ofstream& f) const {
	f << " -jar " << jar << " ";
    }

    virtual void
    write_file_args(std::ofstream& f, vector<string> reldirs) const {
	f << "*.jar";
    }
};

class TOOLD_FINDBUGS074 : TOOLD_FINDBUGS {
public:
    TOOLD_FINDBUGS074()
	: TOOLD_FINDBUGS("0.7.4") {
	add_jar("findbugs-0.7.4.jar");
    }
} td_fb074;

class TOOLD_FINDBUGS131 : TOOLD_FINDBUGS {
public:
    TOOLD_FINDBUGS131()
	: TOOLD_FINDBUGS("1.3.1") {
	add_jar("findbugs-1.3.1.jar");
    }
} td_fb131;



//
// Class to fetch info from a subversion repository.
//
class SVN_RESULT {
public:
    SVN_RESULT(const char * url, const char * username, const char * password);

    bool get_successful() const { return successful; };

    const char * get_rooturl() const { return rooturl.c_str(); };
    const char * get_uuid() const { return uuid.c_str(); };
    int get_last_revision() const { return last_revision; };
    int get_not_changed_in() const { return not_changed_in; };

private:
    bool successful;
    string rooturl;
    string uuid;
    int last_revision;
    int not_changed_in;
};

SVN_RESULT::SVN_RESULT(const char * url,
		       const char * username, const char * password) {
    successful = false;
    last_revision = -1;

    char tempfile[100];
    strcpy(tempfile, "/tmp/scospubWG.XXXXXX");

    char * filename = mktemp(tempfile);

    // TODO: errorcheck!
    string commandline;

    // TODO: Hardcoded svn
    commandline.append("svn info ");
    commandline.append("--no-auth-cache ");
    commandline.append("--non-interactive ");
    commandline.append(url);
    commandline.append(" --username '");
    commandline.append(username);
    commandline.append("' --password '");
    commandline.append(password);
    commandline.append("' > '");
    commandline.append(filename);
    commandline.append("' 2>&1");
    commandline.append(" </dev/null");

    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
			"Fetching svn data from %s\n",
			url);

    SP_LOG_TIME_COND time_for_system(log_messages);

    int result = system(commandline.c_str());

    if (result != 0) {
	log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
			    "Cannot fetch data from %s. "
			    "Error %d.\n",
			    url,
			    result
	    );

	unlink(filename);
	return;
    }

    time_for_system.printf(SCHED_MSG_LOG::MSG_NORMAL, 60,
			   "system(%s) took too long.\n", 
			   commandline.c_str());

    // Parse through the result
    string line;

    std::ifstream file(filename, std::ios::in);
    // TODO: Error check

    while (getline(file, line)) {
#define SVN_LCR "Last Changed Rev: "
	if (line.compare(0, strlen(SVN_LCR), SVN_LCR) == 0) {
	    string sub(line.substr(strlen(SVN_LCR)));
	    last_revision = atoi(sub.c_str());
	}

#define SVN_ROOTURL "Repository Root: "
	if (line.compare(0, strlen(SVN_ROOTURL), SVN_ROOTURL) == 0) {
	    rooturl = line.substr(strlen(SVN_ROOTURL));
	}

#define SVN_UUID "Repository UUID: "
	if (line.compare(0, strlen(SVN_UUID), SVN_UUID) == 0) {
	    uuid = line.substr(strlen(SVN_UUID));
	}

#define SVN_LAST_CHANGED "Last Changed Date: "
	if (line.compare(0, strlen(SVN_LAST_CHANGED), SVN_LAST_CHANGED)
	    == 0) {
	    struct tm when;
	    time_t not_changed_since = time(NULL);

	    int year;
	    int mon;

	    if (sscanf(line.substr(strlen(SVN_LAST_CHANGED)).c_str(), 
		       "%d-%d-%d %d:%d:%d",
		       &year,
		       &mon,
		       &when.tm_mday,
		       &when.tm_hour,
		       &when.tm_min,
		       &when.tm_sec) == 6) {
		when.tm_year = year - 1900;
		when.tm_mon = mon - 1;
		not_changed_since = mktime(&when);
	    }

	    // TODO: Does not account for time differences but makes
	    // a conservative guess.
	    // This means that it doesn't begin backing off until
	    // up to 24 hours after the last commit.
	    if (not_changed_since > 0) {
		not_changed_in = time(NULL) - not_changed_since - 24 * 3600;
	    } else {
		// Information was not received correctly. We don't know
		// anything.
		not_changed_in = 0;
	    }

	    if (not_changed_in < 0) {
		not_changed_in = 0;
	    }
	}
    }
    file.close();
    unlink(filename);

    if (last_revision > 0) {
	successful = true;
    }
}




//
// Process new SVN URLs
int
process_svn_urls() {
    static time_t last = time(NULL);

    // Don't do this more often than every three seconds.
    if (last + 3 > time(NULL)) {
	return 0;
    }

    time(&last);

    DB_SCOS_SOURCE source;

    while (!source.enumerate("WHERE valid='unknown'")) {
	SVN_RESULT res(source.url, source.username, source.password);

	if (!res.get_successful()) {
	    strcpy(source.valid, "invalid");
	} else {
	    strcpy(source.rooturl, res.get_rooturl());
	    strcpy(source.uuid, res.get_uuid());

	    strcpy(source.valid, "valid");
	}

	source.update();
    }
    source.end_enumerate();

    return 0;
}





// Called once for each tool configured for a project when
// the source of the project is updated.
//
int make_job(const SCOS_PROJECT project,
	     const TEAM team,
	     const SCOS_TOOL tool,
	     revisions_map_type svn_revisions) {
    // Files to create:
    // job.xml different for each job
    // input files (any?)

    static int seqno = 1;

    // make a unique name (for the job and its input file)
    //
    // This name is also used by the work assimilation to enter
    // the result in the correct way.
    char name[255];
    sprintf(name,
	    "%s_%s_%d_%d_t%d",
	    tool.name,
	    team.name_lc,
	    start_time,
	    seqno++,
	    tool.id);

    for (revisions_map_type::iterator it = svn_revisions.begin();
	 it != svn_revisions.end();
	 it++) {
	sprintf(name, "%s_s%dr%d", name, (*it).first, (*it).second);
    }

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    char path[255];
    int retval = config.download_path(name, path);
    if (retval) return retval;

    std::ofstream f(path);
    if (!f.is_open()) {
        return ERR_FOPEN;
    }

    f << "<" JOB_TAG ">\n";
    f << "  <" TAG_PROJECT " id='" << project.id <<  "'>"
      << project.name << "</" TAG_PROJECT ">\n";
    f << "  <" TAG_TEAM " id='" << team.id << "'>"
      << team.name_lc << "</" TAG_TEAM ">\n";
    f << "  <" TAG_TOOL " id='" << tool.id << "'>"
      << tool.name << "</" TAG_TOOL ">\n";
    f << "  <" TAG_CONFIG ">" << tool.config << "</" TAG_CONFIG ">\n";

    DB_SCOS_SOURCE source;

    char clause[100];
    sprintf(clause,
	    "WHERE project = %d AND active=true AND valid='valid'",
	    project.id);

    vector<string> reldirs;

    // Mapping a project to url
    f << "  <" TAG_SOURCE ">\n";
    while (!source.enumerate(clause)) {
	reldirs.push_back(source.reldir);

	f << "    <" TAG_SVN " id='" << source.id << "'>\n";
	f << "      <" TAG_URL ">" << source.url << "</" TAG_URL ">\n";

	f << "      <" TAG_CHECKOUTDIR ">" << source.reldir << "</" TAG_CHECKOUTDIR ">\n";
	f << "      <" TAG_USERNAME ">" << source.username << "</" TAG_USERNAME ">\n";
	f << "      <" TAG_PASSWORD ">" << source.password << "</" TAG_PASSWORD ">\n";
	f << "      <" TAG_REVISION ">" << svn_revisions[source.id] << "</" TAG_REVISION ">\n";
	f << "    </" TAG_SVN ">\n";
    }
    f << "  </" TAG_SOURCE ">\n";
    source.end_enumerate();

    f << "  <" TAG_TASK_JAVA ">\n";
    f << "    <" TAG_STDOUT_FILENAME ">";
    f << "out1";
    f << "</" TAG_STDOUT_FILENAME ">\n";

    f << "    <" TAG_COMMAND_LINE ">";

    TOOLDETAILS * tooldp = TOOLDETAILS::lookup(&tool);

    tooldp->write_command_line(f);

    f << "</" TAG_COMMAND_LINE ">\n";

    f << "    <" TAG_ARGS ">";

    tooldp->write_file_args(f, reldirs);

    f << "</" TAG_ARGS ">\n";

    f << "    <" TAG_IGNORE_EXIT ">1</" TAG_IGNORE_EXIT ">\n";
    f << "  </" TAG_TASK_JAVA ">\n";

    // Task to parameterize the result
    // Read the output file sofar and calculate the lines!
    f << "  <" TAG_TASK ">\n";
    f << "    <" TAG_APPLICATION ">/bin/sh</" TAG_APPLICATION ">\n";
    f << "    <" TAG_COMMAND_LINE ">"
	"-c 'echo SCOSPUB_RESULT: `wc -l`'"
	"</" TAG_COMMAND_LINE ">\n";
    f << "    <" TAG_STDIN_FILENAME ">";
    f << "out1";
    f << "</" TAG_STDIN_FILENAME ">\n";
    f << "    <" TAG_STDOUT_FILENAME ">";
    f << "out2";
    f << "</" TAG_STDOUT_FILENAME ">\n";

    f << "  </" TAG_TASK ">\n";

    f << "</" JOB_TAG ">\n";

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

    int num_infiles = 1 + tooldp->get_infiles()->size();
    const char * infiles[num_infiles];
    infiles[0] = name;

    list<string>::const_iterator it = tooldp->get_infiles()->begin();
    for (int i = 1;
	 it != tooldp->get_infiles()->end();
	 it++, i++) {
	infiles[i] = it->c_str();
    }

    string rel_template_filename = "../";
    rel_template_filename += tooldp->get_result_template_filename();

    // Register the job with BOINC
    //
    SP_LOG_TIME_COND time_for_create_work(log_messages);

    create_work(
        wu,
        tooldp->get_wu_template(),
        tooldp->get_result_template_filename(),
        rel_template_filename.c_str(),
        infiles,
        num_infiles,
        config
    );

    time_for_create_work.printf(SCHED_MSG_LOG::MSG_NORMAL, 5,
				"create_work() took too long.\n");
}


int main_loop() {
    int retval;

    // perpetually
    while (1) {
	DB_SCOS_PROJECT project;

	const char * project_clause =
	    "WHERE active = TRUE AND NOW() > nextpoll";

	int waiting_projects;
	retval = project.count(waiting_projects, project_clause);
	if (retval) return retval;

	if (waiting_projects > 10) {
	    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
				"There are %d projects waiting.\n",
				waiting_projects);
	} else if (waiting_projects > 3) {
	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"There are %d projects waiting.\n",
				waiting_projects);
	}

	while (!project.enumerate(project_clause)) {
	    retval = process_svn_urls();
	    if (retval) return retval;

	    DB_TEAM team;

	    retval = team.lookup_id(project.team);
	    if (retval) return retval;

	    string projlog = "";
	    projlog += "project ";
	    projlog += project.name;
	    projlog += " (projid ";
	    {
		char buf[10];
		sprintf(buf, "%d", project.id);
		projlog += buf;
	    }
	    projlog += ") for team ";
	    projlog += team.name;

	    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				"Processing %s\n",
				projlog.c_str()
		    );

	    check_stop_daemons();

	    DB_SCOS_SOURCE source;

	    // TODO: Hardcoded to svn
	    revisions_map_type svn_revisions;

	    int changes = 0;
	    int errors = 0;

	    char clause[100];
	    sprintf(clause,
		    "WHERE project = %d AND active = TRUE AND valid='valid'",
		    project.id);

#define NO_TIME_GOTTEN (10 * 365 * 24 * 3600)
            int not_changed_in = NO_TIME_GOTTEN;

	    while (!source.enumerate(clause)) {
		if (source.type != 1) {
		    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
					"Cannot handle source of type %d "
					"for %s.\n",
					source.type,
					projlog.c_str());
		    errors++;
		    continue;
		}

		SVN_RESULT res(source.url, source.username, source.password);

		if (!res.get_successful()) {
		    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
					"Cannot fetch SVN information for %s "
					"for %s.\n",
					source.url,
					projlog.c_str());
		    errors++;
		    continue;
		}

		if (strcmp(source.rooturl, res.get_rooturl())) {
		    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
					"Rooturl changed from %s to %s "
					"for %s.\n",
					source.rooturl,
					res.get_rooturl(),
					projlog.c_str());
		    strcpy(source.rooturl, res.get_rooturl());
		    source.update();
		}

		if (strcmp(source.uuid, res.get_uuid())) {
		    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
					"UUID changed from %s to %s for %s.\n",
					source.uuid,
					res.get_uuid(),
					projlog.c_str());
		    strcpy(source.uuid, res.get_uuid());
		    source.update();
		}

		if (res.get_not_changed_in() < not_changed_in) {
		    not_changed_in = res.get_not_changed_in();
		}

		svn_revisions[source.id] = res.get_last_revision();

		if (res.get_last_revision() <= source.lastrevision) {
		    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
					"Revision %d is not newer than %d "
					"for %s.\n",
					res.get_last_revision(),
					source.lastrevision,
					projlog.c_str()
			);
		    continue;
		}
		changes++;
	    }
	    source.end_enumerate();

	    if (errors == 0) {
		if (changes > 0) {
		    // There are indeed changes to some of the sources.

		    DB_SCOS_TOOL tool;

		    char toolclause[100];
		    sprintf(toolclause,
			    "WHERE project = %d AND active = TRUE",
			    project.id);

		    while (!tool.enumerate(toolclause)) {
			// Creat one job per tool.
			log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
					    "Creating job %s %s for %s.\n",
					    tool.name,
					    tool.config,
					    projlog.c_str()
			    );
			make_job(project, team, tool, svn_revisions);
		    }

		    while (!source.enumerate(clause)) {
			source.lastrevision = svn_revisions[source.id];
			source.update();
		    }
		    source.end_enumerate();
		}

		int outstanding_wus;
		retval = count_workunits(outstanding_wus,
					 "WHERE canonical_resultid = 0 "
					 "AND error_mask = 0");
		if (retval) return retval;

		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "Processing %s done. Outstanding WUs: %d.\n",
				    projlog.c_str(),
				    outstanding_wus
		    );
	    } else {
		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "Processing %s aborted. %d errors seen.\n",
				    projlog.c_str(),
				    errors
		    );
	    }

	    // Wait a while between every polling.
	    // TODO: Tune this.
#define MIN_DELAY (60)
	    project.delay = MIN_DELAY;

	    // Wait longer if the repository hasn't been recently updated
	    if (not_changed_in == NO_TIME_GOTTEN) {
		log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
				    "No source was accessible for %s.\n",
				    projlog.c_str());
	    } else if (not_changed_in > 24 * 3600) {
		// If it was a long time since this project was updated
		// don't immediately poll it again.
		// 25 hours will cause a poll delay around two minutes.
		// This is exponentially decreasing with a max delay of
		// almost two hours.
		// We never delay more than 2 hour
#define MAX_DELAY (7200)
#define FACTOR (5000000.0)
		double del = not_changed_in - 24 * 3600;

		int new_delay = (int) trunc(((double) MAX_DELAY)
					    * (1 - exp(- del / FACTOR)));

		if (new_delay > MAX_DELAY) {
		    new_delay = MAX_DELAY;
		}

		if (new_delay > MIN_DELAY) {
		    project.delay = new_delay;
		    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
					"Delaying next processing "
					"for %02d:%02d:%02d "
					"for %s. "
					"Youngest source was "
					"%d days and %d hours.\n",
					new_delay / 3600,
					(new_delay / 60) % 60,
					new_delay % 60,
					projlog.c_str(),
					not_changed_in / 3600 / 24,
					(not_changed_in / 3600) % 24
					);
		}
	    }

	    project.update();
	}

	retval = process_svn_urls();
	if (retval) return retval;
	sleep(10);
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

    SP_LOG_TIME_COND time_db_open(log_messages);
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't open db\n");
        exit(1);
    }
    time_db_open.printf(SCHED_MSG_LOG::MSG_NORMAL, 60,
			"Opening the database took too long.\n");

    // TODO: Hardcoded!
    if (app.lookup("where name='scospubapp'")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't find app\n");
        exit(1);
    }

    start_time = time(0);

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    retval = main_loop();
    if (retval) {
	log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
			    "Exiting on error.\n");
    }
}
