#include "boinc_db.h"

// class to represent a record from the scos_project table
struct SCOS_PROJECT {
    int id;
    char name[256];
    char user_friendly_name[256];
    int active;

    void clear();
};

struct SCOS_SOURCE {
    int id;
    int project;
    int type;

    // Static data
    // Subversion
    char url[256];
    char username[35];
    char password[35];

    char rooturl[256];
    char uuid[100];
    
    // CVS:
    // Other...

    // Dynamic data
    int lastrevision;

    int active;

    void clear();
};


// class to represent a record from the scos_tool table
struct SCOS_TOOL {
    int id;
    int project;
    char name[256];
    char config[256];
    int active;

    void clear();
};

// class to represent a record from the scos_result table
struct SCOS_RESULT {
    int id;
    int create_time;
    // date is skipped
    int tool;
    int result;
    char file[256];

    void clear();
};

// class to connect the result with the tools
struct SCOS_RESULT_SOURCE {
    int source;
    int result;

    // Subversion
    int revision;
    // CVS
    char date[256];
    // Other
    // ...

    int active;

    void clear();
};    

#define DBCLASS(type) \
    class DB_SCOS_ ## type : public DB_BASE, public SCOS_ ## type { \
    public: \
	DB_SCOS_ ## type(DB_CONN* p = 0); \
	int get_id(); \
	void db_print(char*); \
	void db_parse(MYSQL_ROW &row); \
    }

DBCLASS(PROJECT);
DBCLASS(SOURCE);
DBCLASS(TOOL);
DBCLASS(RESULT);
DBCLASS(RESULT_SOURCE);
