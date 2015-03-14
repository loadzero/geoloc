/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This file contains the logging and error handling implementation. We use a 
 * mirrored ring buffer to handle log messages. The ring buffer is dumped out 
 * to stderr when an assert or fatal error fires.
 * 
 * Note - log messages larger than 4095 bytes will get truncated to 4095 bytes.
*/

#include "error.hpp"
#include <string.h>
#include <assert.h>

static char error_buf_[8192] = {0};
static size_t error_offset_ = 0;
static size_t avail_ = 0;
static char print_buf_[4096] = {0};

// note - log messages larger than 4095 bytes will get truncated to 4095.

void log_context(const char* file, unsigned line, const char* fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n = vsnprintf(print_buf_, 4096, fmt, ap);
    va_end(ap);

    if (n > 4095) n = 4095;

    // replace the nul terminator with a new line

    print_buf_[n] = '\n';

    memcpy(error_buf_ + error_offset_, print_buf_, n + 1);
    memcpy(error_buf_ + error_offset_ + 4096, print_buf_, n + 1);

    error_offset_ += n+1;
    error_offset_ = error_offset_ % 4096;

    avail_ += (n+1);

    if (avail_ > 4096)
    {
        avail_ = 4096;
    }
}

static void log_dump()
{
    const char* iter = error_buf_ + error_offset_ + (4096 - avail_);
    fprintf(stderr, "%s", iter);
}

void fatal_error(const char* file, unsigned line, const char* fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d: error: ", file, line);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    if (avail_)
    {
        fprintf(stderr, "context:\n");
        log_dump();
    }

    exit(1);
}
