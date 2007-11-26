#include <fstream>

#include "filesys.h"

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

    if (wu.canonical_resultid)
    {
        vector<string> output_file_paths;
        get_output_file_paths(canonical_result, output_file_paths);

	// TODO: Hardcoded count of files.
	if (output_file_paths.size() != 3)
	{
	    // TODO: Logg the error.
	    return 1;
	}

	// Lets initially assume that the files are always in the
	// right order.

	// out1:
	// Line 1: project, rev
	// Line 2: toolid tool config
	std::ifstream f1(output_file_paths[0].c_str());

	string osproj;
	int rev;

	// TODO: Felhantering
	// TODO: Flera olika revisions.
	f1 >> osproj >> rev;

	int toolid;
	string tool;
	string config;

	// TODO: Felhantering
	f1 >> toolid >> tool >> config;

	f1.close();

#define SCOSDATA "../html/scosdata"
	// out2:
	int retval = boinc_mkdir(SCOSDATA);
	if (retval)
	{
	    // TODO: Logg the error.
	    return retval;
	}

	string copy_path(SCOSDATA);

	copy_path += "/";
	copy_path += wu.name;

	string path = output_file_paths[1];
	
	retval = boinc_copy(path.c_str() , copy_path.c_str());
	if (retval)
	{
	    // TODO: Logg the error.
	    return retval;
	}

	// out3:
	std::ifstream f3(output_file_paths[2].c_str());

	int result;
	f3 >> result;

	f3.close();


	DB_SCOS_RESULT res;
	res.create_time = canonical_result.received_time;
	res.revision = rev;
	res.tool = toolid;
	res.result = result;
	strcpy(res.file, copy_path.c_str());

	res.insert();
	// TODO: Error handler.

	return 0;
    }

    return 1;
}
