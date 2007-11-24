#include "assimilate_handler.h"

using std::vector;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
)
{
    if (wu.error_mask && wu.canonical_resultid == 0) {
        return 0;
    }

    return 1;
}
