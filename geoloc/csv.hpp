/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This file contains utility functions for tokenizing and parsing strings.
*/

#ifndef CSV_HPP_BE8C5A6D
#define CSV_HPP_BE8C5A6D

inline unsigned to_u(const char* s)
{
    return strtoul(s, 0, 10);
}

inline void csv_split(const char* s,
                      size_t n,
                      std::string &scratch,
                      std::vector<char*> &toks)
{
    toks.clear();
    scratch.clear();

    scratch.assign(s, n);

    // csv splitter needs to handle quote marks.
 
    char* ptr = &scratch[0];
    bool first = true;
    char* begin = 0;

    while (*ptr)
    {
        if (!first)
        {
            *ptr = '\0';
            ++ptr;
        }
        else
        {
            first = false;
        }

        if (*ptr != '"')
        {
            begin = ptr;
            while (*ptr && *ptr != ',') ++ptr;
            toks.push_back(begin);
        }
        else
        {
            // skip quote
            ++ptr;
            begin = ptr;

            while (*ptr && *ptr != '"') ++ptr;

            if (*ptr == '"')
            {
                *ptr = '\0';
                ++ptr;
            }

            toks.push_back(begin);
        }
    }
}

inline void char_split(const std::string &s,
                      std::string &scratch,
                      std::vector<char*> &toks,
                      char delim)
{
    toks.clear();
    scratch.clear();

    scratch.assign(s.begin(), s.end());

    char* ptr = &scratch[0];
    bool first = true;

    while (*ptr)
    {
        if (!first)
        {
            first = false;
            *ptr = '\0';
            ++ptr;
        }
        else
        {
            first = false;
        }

        char* begin = ptr;
        while (*ptr && *ptr != delim) ++ptr;

        toks.push_back(begin);
    }
}

#endif
