// SCOSPUB - tools supporting Open Source development.
// Copyright (C) 2008 Linus Tolke
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option)
// any later version.

#include <time.h>

#include "sched_msgs.h"

class SP_LOG_TIME_COND {
private:
    SCHED_MSG_LOG& log;
    time_t started;

public:
    SP_LOG_TIME_COND(SCHED_MSG_LOG& l) : log(l) { started = time(NULL); }

    void printf(int kind, int limit, const char* format, ...) const
	__attribute__ ((format (printf, 4, 5))) {
	va_list va;
	va_start(va, format);
	vprintf(kind, limit, format, va);
	va_end(va);
    };

    void vprintf(int kind, int limit, const char* format, va_list va) const {
	long t = time(NULL) - started;
	if (t > limit) {
	    log.vprintf(kind, format, va);
	    log.printf(kind,
		       "Took %lds. Should not have taken more than %ds.\n",
		       t, limit);
	}
    };
};

