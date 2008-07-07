// SCOSPUB - tools supporting Open Source development.
// Copyright (C) 2008 Linus Tolke
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

#include <string>

#include "parse.h"

class SX_PARSER : public XML_PARSER {
private:
    bool parse_str(const char* parsed_tag, const char* start_tag,
		   int& id,
		   char* buf, int len);
    static bool match_tag(const char * parsed, const char * start_tag,
			  int& id,
			  bool& is_end_tag, char * buf);

public:
    SX_PARSER(MIOFILE*);

    bool parse_string(const char *, const char *, std::string&, int& id);
    bool parse_string(const char * tag, const char * st, std::string& str);

    static bool match_tag(const char * parsed, const char * start_tag,
			  int& id);
};
