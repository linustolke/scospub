#include <fstream>

#include "filesys.h"

#include "sched_msgs.h"
#include "assimilate_handler.h"
#include "validate_util.h"

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

#define MAX_FILE_NAME_LEN 1025
	char toolfilename[MAX_FILE_NAME_LEN];
	char projectname[MAX_FILE_NAME_LEN];
	int toolid;
	int rev;
	int wgstart;

	if (sscanf(wu.name, "%[^_-]_%[^_-]-%d-%d_%d",
		   toolfilename,
		   projectname,
		   &rev,
		   &toolid,
		   &wgstart
		   ) != 5)
	{
	    log.printf("Cannot parse information from unit name %s.\n",
		       wu.name);
	    return 1;
	}

	// Lets initially assume that the files are always in the
	// right order.

	// The first file is not currently interesting.

#define SCOSDATA "../html/scosdata"
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
	res.revision = rev;
	res.tool = toolid;
	res.result = result;
	strncpy(res.file, wu.name, 250);

	res.insert();
	// TODO: Error handler.

	return 0;
    }

    return 1;
}
