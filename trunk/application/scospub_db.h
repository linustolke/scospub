#include "boinc_db.h"

// class to represent a record from the scos_project table
struct SCOS_PROJECT {
    int id;
    char name[256];
    int active;
    char user_friendly_name[256];

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
    int revision;
    int tool;
    int result;
    char file[256];

    void clear();
};

class DB_SCOS_PROJECT : public DB_BASE, public SCOS_PROJECT {
public:
    DB_SCOS_PROJECT(DB_CONN* p = 0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_SCOS_TOOL : public DB_BASE, public SCOS_TOOL {
public:
    DB_SCOS_TOOL(DB_CONN* p = 0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_SCOS_RESULT : public DB_BASE, public SCOS_RESULT {
public:
    DB_SCOS_RESULT(DB_CONN* p = 0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};
