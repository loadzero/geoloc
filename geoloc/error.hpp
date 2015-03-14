/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This file contains the declarations needed to support logging and error 
 * handling.
*/

#ifndef ERROR_HPP_B43A43DE
#define ERROR_HPP_B43A43DE

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void fatal_error(const char* file, unsigned line, const char* fmt, ...);
void log_context(const char* file, unsigned line, const char* fmt, ...);

#define REL_ASSERT(condition) { if (!(condition)) fatal_error(__FILE__, __LINE__, "assert failed (" #condition ")"); }
#define FATAL_ERROR(...) { fatal_error(__FILE__, __LINE__, __VA_ARGS__); }
#define LOG_CONTEXT(...) { log_context(__FILE__, __LINE__, __VA_ARGS__); }

#endif
