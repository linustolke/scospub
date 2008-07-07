// SCOSPUB - tools supporting Open Source development.
// Copyright (C) 2007, 2008 Linus Tolke
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option)
// any later version.
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

int DB_SCOS_PROJECT::get_id() {return id;}
int DB_SCOS_SOURCE::get_id() {return id;}
int DB_SCOS_TOOL::get_id() {return id;}
int DB_SCOS_RESULT::get_id() {return id;}
int DB_SCOS_RESULT_SOURCE::get_id() {return 0;}

// db_print method for the scos_project table
void DB_SCOS_PROJECT::db_print(char* buf) {
    char name2[2 * sizeof name];

    safe_strcpy(name2, name);

    ESCAPE(name2);

    sprintf(buf,
	    "team=%d, "
	    "name='%s', "
	    "active=%d, "
	    "nextpoll=NOW() + INTERVAL %d SECOND",
	    team,
	    name,
	    active,
	    delay
	);
}

// db_parse method for the scos_project table
void DB_SCOS_PROJECT::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    team = atoi(r[i++]);
    strcpy2(name, r[i++]);
    strcpy2(nextpoll, r[i++]);
    active = atoi(r[i++]);
    delay = 0; // We don't read the delay.
}

// db_print method for the scos_source table
void DB_SCOS_SOURCE::db_print(char * buf) {
    char reldir2[2 * sizeof reldir];
    char url2[2 * sizeof url];
    char username2[2 * sizeof username];
    char password2[2 * sizeof password];
    char rooturl2[2 * sizeof rooturl];
    char uuid2[2 * sizeof uuid];
    char valid2[2 * sizeof valid];

    safe_strcpy(reldir2, reldir);
    safe_strcpy(url2, url);
    safe_strcpy(username2, username);
    safe_strcpy(password2, password);
    safe_strcpy(rooturl2, rooturl);
    safe_strcpy(uuid2, uuid);
    safe_strcpy(valid2, valid);

    ESCAPE(reldir2);
    ESCAPE(url2);
    ESCAPE(username2);
    ESCAPE(password2);
    ESCAPE(rooturl2);
    ESCAPE(uuid2);
    ESCAPE(valid2);

    sprintf(buf,
	    "project=%d, "
	    "reldir='%s', "
	    "type=%d, "
	    "url='%s', "
	    "username='%s', "
	    "password='%s', "
	    "rooturl='%s', "
	    "uuid='%s', "
	    "lastrevision=%d, "
	    "valid='%s', "
	    "active=%d",

	    project,
	    reldir,
	    type,
	    url2,
	    username2,
	    password2,
	    rooturl2,
	    uuid2,
	    lastrevision,
	    valid2,
	    active);
};


// db_parse method for the scos_source table
void DB_SCOS_SOURCE::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    project = atoi(r[i++]);
    strcpy2(reldir, r[i++]);
    type = atoi(r[i++]);
    strcpy2(url, r[i++]);
    strcpy2(username, r[i++]);
    strcpy2(password, r[i++]);
    strcpy2(rooturl, r[i++]);
    strcpy2(uuid, r[i++]);
    lastrevision = atoi(r[i++]);
    strcpy2(valid, r[i++]);
    active = atoi(r[i++]);
}


// db_print method for the scos_tool table
void DB_SCOS_TOOL::db_print(char* buf) {
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
void DB_SCOS_RESULT::db_print(char* buf) {
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
void DB_SCOS_RESULT::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    id = atoi(r[i++]);
    create_time = atoi(r[i++]);
    tool = atoi(r[i++]);
    result = atoi(r[i++]);
    strcpy2(file, r[i++]);
}


// db_print method for the scos_result table
void DB_SCOS_RESULT_SOURCE::db_print(char* buf) {
    sprintf(buf,
	    "source=%d, "
	    "result=%d, "
	    "revision=%d",
	    source,
	    result,
	    revision
	);
}

// db_parse method for the scos_result table
void DB_SCOS_RESULT_SOURCE::db_parse(MYSQL_ROW& r) {
    int i = 0;
    clear();
    source = atoi(r[i++]);
    result = atoi(r[i++]);
    revision = atoi(r[i++]);
}
