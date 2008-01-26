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

#include "boinc_db.h"

// class to represent a record from the scos_project table
struct SCOS_PROJECT {
    int id;
    int team;
    int active;
    int delay;

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

    char valid[10];
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
    // Other
    // ...

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
