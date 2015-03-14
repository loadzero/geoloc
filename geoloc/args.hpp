/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-13
 *
 * This file contains a command line argument helper class.
*/

#ifndef ARGS_HPP_B4B54EC4
#define ARGS_HPP_B4B54EC4

struct Args
{
    Args(int argc, char** argv)
        :
        argc_(argc),
        argv_(argv),
        idx_(1)
    {
    }

    const char* peek()
    {
        if (empty())
        {
            return 0;
        }

        return argv_[idx_];
    }

    const char* pop()
    {
        if (empty())
        {
            return 0;
        }

        const char* ret = argv_[idx_];
        idx_++;

        return ret;
    }

    bool empty()
    {
        return idx_ == argc_;
    }

    int argc_;
    char** argv_;
    int idx_;
};

#endif
