/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-13
 *
 * This file contains macros that are commonly used throughout the source.
*/

#ifndef MACROS_HPP_942AF38F
#define MACROS_HPP_942AF38F

#define UNUSED(x) (void)(x)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);   \
  void operator=(const TypeName&)

inline const char* get_endian()
{
    union
    {
        unsigned u;
        char s[4];
    } data;

    data.u = 0x01020304;

    return data.s[0] == 0x04 ? "little" : "big";
}

#endif
