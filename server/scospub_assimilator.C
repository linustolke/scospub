#include <fstream>
#include <map>

#include "filesys.h"

#include "sched_msgs.h"
#include "assimilate_handler.h"
#include "validate_util.h"
#include "str_util.h"

#include "scospub_db.h"

using std::vector;
using std::string;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
)
{
    if (wu.error_mask && wu.canonical_resultid == 0)
    {
        return 0;
    }

    SCOPE_MSG_LOG log(log_messages, SCHED_MSG_LOG::MSG_NORMAL);

    if (wu.canonical_resultid)
    {
        vector<string> output_file_paths;
        get_output_file_paths(canonical_result, output_file_paths);

	// TODO: Hardcoded count of files.
#define COUNT_OUTPUT_FILES 2
	if (output_file_paths.size() != COUNT_OUTPUT_FILES)
	{
	    log.printf("Not right count of files: %d != %d\n",
		       output_file_paths.size(),
		       COUNT_OUTPUT_FILES);
	    return 1;
	}

	char wuname[1025];

	safe_strcpy(wuname, wu.name);
	char * stoksave;
	char * toolfilename = strtok_r(wuname, "_", &stoksave);
#define ERRORRET(ARG) if (ARG) { \
            log.printf("Cannot parse information from unit name %s.\n", \
		       wu.name); \
            return 1; \
	}
	ERRORRET(toolfilename == NULL);

	char * projectname = strtok_r(NULL, "_", &stoksave);
	ERRORRET(projectname == NULL);

	char * wgstartcp = strtok_r(NULL, "_", &stoksave);
	ERRORRET(wgstartcp == NULL);

	char * seqnocp = strtok_r(NULL, "_", &stoksave);
	ERRORRET(seqnocp == NULL);

	char * toolidcp = strtok_r(NULL, "_", &stoksave);
	ERRORRET(toolidcp == NULL);

	int toolid = atoi(toolidcp + 1);

	std::map<const int, int> revisions;

	char * sourcecp;
	char * revisioncp;
	int source;
	int revision;
	while ((sourcecp = strtok_r(NULL, "r", &stoksave)) != NULL
	       && sourcecp[0] == 's'
	       && (source = atoi(sourcecp + 1)) > 0
	       && (revisioncp = strtok_r(NULL, "_", &stoksave)) != NULL
	       && (revision = atoi(revisioncp)) > 0
	    ) {
	    revisions[source] = revision;
	}

	// Lets initially assume that the files are always in the
	// right order.

	// The first file is not currently interesting.

#define SCOSDATA "../html/scospres/DATA"
	// out1:
	int retval = boinc_mkdir(SCOSDATA);
	if (retval)
	{
	    log.printf("Cannot create the directory %s\n", SCOSDATA);
	    return retval;
	}

	string copy_path(SCOSDATA);

	copy_path += "/";
	copy_path += wu.name;

	string path = output_file_paths[0];
	
	retval = boinc_copy(path.c_str() , copy_path.c_str());
	if (retval)
	{
	    log.printf("Cannot copy the result file to %s.\n",
		       copy_path.c_str());
	    return retval;
	}

	// out2:
	std::ifstream f3(output_file_paths[1].c_str());

	int result;
	f3 >> result;

	f3.close();


	DB_SCOS_RESULT res;
	res.create_time = canonical_result.received_time;
	res.tool = toolid;
	res.result = result;
	strncpy(res.file, wu.name, 250);

	res.insert();
	// TODO: Error handler.

	res.id = res.db->insert_id();

	for (std::map<const int, int>::iterator it = revisions.begin();
	     it != revisions.end();
	     it++)
	{
	    DB_SCOS_RESULT_SOURCE dsrs;

	    dsrs.source = (*it).first;
	    dsrs.result = res.id;
	    dsrs.revision = (*it).second;

	    dsrs.insert();
	}

	return 0;
    }

    return 1;
}