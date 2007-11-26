#include "str_util.h"

#include "scospub_db.h"

static inline void
ESCAPE(char * x)
{
    escape_string(x, sizeof(x));
}
static inline void
UNESCAPE(char * x)
{
    unescape_string(x, sizeof(x));
}

void SCOS_PROJECT::clear() {memset(this, 0, sizeof(*this));}
void SCOS_TOOL::clear() {memset(this, 0, sizeof(*this));}
void SCOS_RESULT::clear() {memset(this, 0, sizeof(*this));}

DB_SCOS_PROJECT::DB_SCOS_PROJECT(DB_CONN* dc) :
    DB_BASE("scos_project", dc?dc:&boinc_db) {}
DB_SCOS_TOOL::DB_SCOS_TOOL(DB_CONN* dc) :
    DB_BASE("scos_tool", dc?dc:&boinc_db) {}
DB_SCOS_RESULT::DB_SCOS_RESULT(DB_CONN* dc) :
    DB_BASE("scos_result", dc?dc:&boinc_db) {}

int DB_SCOS_PROJECT::get_id() {return id;}
int DB_SCOS_TOOL::get_id() {return id;}
int DB_SCOS_RESULT::get_id() {return id;}

// db_print method for the scos_project table
void DB_SCOS_PROJECT::db_print(char* buf)
{
    ESCAPE(name);
    ESCAPE(user_friendly_name);

    sprintf(buf,
	    "name='%s', "
	    "active=%d, "
	    "user_friendly_name='%s'",
	    name,
	    active,
	    user_friendly_name
	);

    UNESCAPE(user_friendly_name);
    UNESCAPE(name);
}

// db_parse method for the scos_project table
void DB_SCOS_PROJECT::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
    active = atoi(r[i++]);
    strcpy2(user_friendly_name, r[i++]);
}


// db_print method for the scos_tool table
void DB_SCOS_TOOL::db_print(char* buf)
{
    ESCAPE(name);
    ESCAPE(config);

    sprintf(buf,
	    "project=%d, "
	    "name='%s', "
	    "config='%s', "
	    "active=%d, ",
	    project,
	    name,
	    config,
	    active
	);

    UNESCAPE(config);
    UNESCAPE(name);
}

// db_parse method for the scos_tool table
void DB_SCOS_TOOL::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    project = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(config, r[i++]);
    active = atoi(r[i++]);
}

// db_print method for the scos_result table
void DB_SCOS_RESULT::db_print(char* buf)
{
    ESCAPE(file);

    sprintf(buf,
	    "create_time=%d, "
	    "revision=%d, "
	    "tool='%s', "
	    "result=%d, "
	    "file='%s'",
	    create_time,
	    revision,
	    tool,
	    result,
	    file
	);

    UNESCAPE(file);
}

// db_parse method for the scos_result table
void DB_SCOS_RESULT::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    revision = atoi(r[i++]);
    tool = atoi(r[i++]);
    result = atoi(r[i++]);
    strcpy2(file, r[i++]);
}

