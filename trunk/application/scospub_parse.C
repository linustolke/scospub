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

#include <stdio.h>
#include <stdlib.h>

#include "str_util.h"

#include "scospub_parse.h"

#define MATCH_ID_START " id='"

sx_parser::sx_parser(MIOFILE* f)
    : XML_PARSER(f)
{
}


bool
sx_parser::parse_string(const char * parsed_tag, const char * st,
			std::string& str) {
    char tag[256];
    strcpy(tag, parsed_tag);
    return XML_PARSER::parse_string(tag, st, str);
}

//
// We just parsed "parsed_tag" or "parsed_tag id='NN'".
// If it matches "start_tag", and is followed by a string
// and by the matching close tag, return the string in "buf",
// and return true.
//
bool
sx_parser::parse_str(const char* parsed_tag, const char* start_tag,
		     int& id,
		     char* buf, int len)
{
    bool is_tag, eof;
    char end_tag[256], tag[256], tmp[64000];

    bool is_end_tag;

    if (!match_tag(parsed_tag, start_tag, id, is_end_tag, buf))
    {
	return false;
    }

    if (is_end_tag)
    {
	return true;
    }

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    // get text after start tag
    //
    eof = get(tmp, 64000, is_tag);
    if (eof) return false;

    // if it's the end tag, return empty string
    //
    if (is_tag) {
        if (strcmp(tmp, end_tag)) {
            return false;
        } else {
            strcpy(buf, "");
            return true;
        }
    }

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    strlcpy(buf, tmp, len);
    return true;
}


bool
sx_parser::parse_string(const char * parsed_tag, const char * start_tag,
			std::string& str, int& id)
{
    char buf[8192];
    bool flag = parse_str(parsed_tag, start_tag, id, buf, sizeof(buf));
    if (!flag) return false;
    str = buf;
    return true;
}

bool
sx_parser::match_tag(const char * parsed, const char * start_tag,
		     int& id) {
    bool dont_care;
    return match_tag(parsed, start_tag, id, dont_care, NULL);
}

//
// Match a tag either with or wihtout id argument
bool
sx_parser::match_tag(const char * parsed, const char * start_tag,
		     int& id,
		     bool& is_end_tag,
		     char * buf)
{
    if (strncmp(parsed, start_tag, strlen(start_tag)) != 0)
    {
	return false;
    }

    id = 0;
    is_end_tag = false;
    switch (parsed[strlen(start_tag)])
    {
    case 0:
	break;
    case ' ':
    {
	// There is an argument
	const char * args = parsed + strlen(start_tag);

	if (strncmp(args, MATCH_ID_START, strlen(MATCH_ID_START)) != 0)
	{
	    const char * idarg = args + strlen(MATCH_ID_START);
	    id = atoi(idarg);

	    if (args[strlen(args) - 1] == '/')
	    {
		// The tag has the form <tag id='NN'/>
		is_end_tag = true;
		if (buf != NULL)
		{
		    strcpy(buf, "");
		}
		return true;
	    }
	}
	break;
    }

    case '/':
	// handle the form <tag/>
	is_end_tag = true;
	if (buf != NULL)
	{
	    strcpy(buf, "");
	}
        return true;

    default:
	return false;
    }

    return true;
}
