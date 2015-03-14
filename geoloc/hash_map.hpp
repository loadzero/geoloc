/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This file contains macros to paper over differences between older/newer 
 * compilers as far as using std::unordered_map is concerned.
*/

#ifndef HASH_MAP_HPP_182989E8
#define HASH_MAP_HPP_182989E8

#ifndef __linux
#include <unordered_map>

#define hash_map std::unordered_map

#else
#include <tr1/unordered_map>
#define hash_map std::tr1::unordered_map
#endif

#endif
