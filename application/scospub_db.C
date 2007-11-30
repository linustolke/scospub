#include "str_util.h"

#include "scospub_db.h"

#define ESCAPE(x) escape_string(x, sizeof(x))
#define UNESCAPE(x) unescape_string(x, sizeof(x))

void SCOS_PROJECT::clear() {memset(this, 0, sizeof(*this));}
void SCOS_SOURCE::clear() {memset(this, 0, sizeof(*this));}
void SCOS_TOOL::clear() {memset(this, 0, sizeof(*this));}
void SCOS_RESULT::clear() {memset(this, 0, sizeof(*this));}
void SCOS_RESULT_SOURCE::clear() {memset(this, 0, sizeof(*this));}

DB_SCOS_PROJECT::DB_SCOS_PROJECT(DB_CONN* dc) :
    DB_BASE("scos_project", dc?dc:&boinc_db) {}
DB_SCOS_SOURCE::DB_SCOS_SOURCE(DB_CONN* dc) :
    DB_BASE("scos_source", dc?dc:&boinc_db) {}
DB_SCOS_TOOL::DB_SCOS_TOOL(DB_CONN* dc) :
    DB_BASE("scos_tool", dc?dc:&boinc_db) {}
DB_SCOS_RESULT::DB_SCOS_RESULT(DB_CONN* dc) :
    DB_BASE("scos_result", dc?dc:&boinc_db) {}
DB_SCOS_RESULT_SOURCE::DB_SCOS_RESULT_SOURCE(DB_CONN* dc) :
    DB_BASE("scos_result_source", dc?dc:&boinc_db) {}

int DB_SCOS_PROJECT::get_id() const {return id;}
int DB_SCOS_SOURCE::get_id() const {return id;}
int DB_SCOS_TOOL::get_id() const {return id;}
int DB_SCOS_RESULT::get_id() const {return id;}
int DB_SCOS_RESULT_SOURCE::get_id() const {return 0;}

// db_print method for the scos_project table
void DB_SCOS_PROJECT::db_print(char* buf) const
{
    char name2[2 * sizeof name];
    char user_friendly_name2[2 * sizeof name];
    
    safe_strcpy(name2, name);
    safe_strcpy(user_friendly_name2, user_friendly_name);

    ESCAPE(name2);
    ESCAPE(user_friendly_name2);

    sprintf(buf,
	    "name='%s', "
	    "active=%d, "
	    "user_friendly_name='%s'",
	    name2,
	    active,
	    user_friendly_name2
	);
}

// db_parse method for the scos_project table
void DB_SCOS_PROJECT::db_parse(MYSQL_ROW& r)
{
    int i = 0;
    clear();
    id = atoi(r[i++]);
    strcpy2(name, r[i++]);
    active = atoi(r[i++]);
    strcpy2(user_friendly_name, r[i++]);
}

// db_print method for the scos_source table
void DB_SCOS_SOURCE::db_print(char * buf) const
{
    char url2[2 * sizeof url];
    char rooturl2[2 * sizeof rooturl];
    char uuid2[2 * sizeof uuid];

    safe_strcpy(url2, url);
    safe_strcpy(rooturl2, rooturl);
    safe_strcpy(uuid2, uuid);

    ESCAPE(url2);
    ESCAPE(rooturl2);
    ESCAPE(uuid2);

    sprintf(buf,
	    "project=%d, "
	    "type=%d, "
	    "url='%s', "
	    "rooturl='%s', "
	    "uuid='%s', "
	    "active=%d",
	    project,
	    type,
	    url2,
	    rooturl2,
	    uuid2,
	    active);
};


// db_parse method for the scos_source table
void DB_SCOS_SOURCE::db_parse(MYSQL_ROW& r)
{
    int i = 0;
    clear();
    id = atoi(r[i++]);
    project = atoi(r[i++]);
    type = atoi(r[i++]);
    strcpy2(url, r[i++]);
    strcpy2(rooturl, r[i++]);
    strcpy2(uuid, r[i++]);
    active = atoi(r[i++]);
}


// db_print method for the scos_tool table
void DB_SCOS_TOOL::db_print(char* buf) const
{
    char name2[2 * sizeof name];
    char config2[2 * sizeof config];
    
    safe_strcpy(name2, name);
    safe_strcpy(config2, config);

    ESCAPE(name2);
    ESCAPE(config2);

    sprintf(buf,
	    "project=%d, "
	    "name='%s', "
	    "config='%s', "
	    "active=%d, ",
	    project,
	    name2,
	    config2,
	    active
	);
}

// db_parse method for the scos_tool table
void DB_SCOS_TOOL::db_parse(MYSQL_ROW& r)
{
    int i = 0;
    clear();
    id = atoi(r[i++]);
    project = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(config, r[i++]);
    active = atoi(r[i++]);
}

// db_print method for the scos_result table
void DB_SCOS_RESULT::db_print(char* buf) const
{
    char file2[2 * sizeof file];

    safe_strcpy(file2, file);

    ESCAPE(file2);

    sprintf(buf,
	    "create_time=%d, "
	    "tool=%d, "
	    "result=%d, "
	    "file='%s', "
	    "date=NOW() ",
	    create_time,
	    tool,
	    result,
	    file2
	);
}

// db_parse method for the scos_result table
void DB_SCOS_RESULT::db_parse(MYSQL_ROW& r)
{
    int i = 0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    tool = atoi(r[i++]);
    result = atoi(r[i++]);
    strcpy2(file, r[i++]);
}


// db_print method for the scos_result table
void DB_SCOS_RESULT_SOURCE::db_print(char* buf) const
{
    char date2[2 * sizeof date];

    safe_strcpy(date2, date);

    ESCAPE(date2);

    sprintf(buf,
	    "source=%d, "
	    "result=%d, "
	    "revision=%d, "
	    "date='%s', "
	    "active=%d, ",
	    source,
	    result,
	    revision,
	    date2,
	    active
	);
}

// db_parse method for the scos_result table
void DB_SCOS_RESULT_SOURCE::db_parse(MYSQL_ROW& r)
{
    int i = 0;
    clear();
    source = atoi(r[i++]);
    result = atoi(r[i++]);
    revision = atoi(r[i++]);
    strcpy2(date, r[i++]);
    active = atoi(r[i++]);
}
