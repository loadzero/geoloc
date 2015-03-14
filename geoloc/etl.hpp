/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module contains helper functions to extract, transform and load a 
 * MaxMind csv dataset.
*/

#ifndef ETL_HPP_89C643E4
#define ETL_HPP_89C643E4

#include "serialization.hpp"
#include "pipeline.hpp"
#include "locations.hpp"
#include "blocks.hpp"
#include "asns.hpp"

inline void build_locations(BinaryFile &file, const char* source)
{
    LOG_CONTEXT("build_locations from %s", source);

    FileReader reader(source);
    LocationParser parser;

    std::vector<Location> locations;
    Collector<Location> collector(locations);

    reader | parser | collector;
    reader.produce();

    save_locations(file, locations);
}

inline void build_blocks(BinaryFile &file, const char* source)
{
    LOG_CONTEXT("build_blocks from %s", source);

    FileReader reader(source);
    BlockParser parser;

    std::vector<Block> blocks;
    Collector<Block> collector(blocks);

    reader | parser | collector;
    reader.produce();

    save_blocks(file, blocks);
}

inline void build_asns(BinaryFile &file, const char* source)
{
    LOG_CONTEXT("build_asns from %s", source);

    FileReader reader(source);
    ASNParser parser;

    std::vector<ASN> asns;
    Collector<ASN> collector(asns);

    reader | parser | collector;
    reader.produce();

    save_asns(file, asns);
}

inline void build_geo_data(BinaryFile &file, 
                           const char* city_blocks, 
                           const char* city_locs,
                           const char* geo_asns)
{
    build_blocks(file, city_blocks);
    build_locations(file, city_locs);
    build_asns(file, geo_asns);
}

inline void get_header(char* buf, size_t n)
{
    REL_ASSERT(n > 0);

    memset(buf, '-', n);

    int np = snprintf(buf, n, "geoloc loadzero v001 %s ", get_endian());

    REL_ASSERT(np >= 0);

    if (np > (int) (n-1))
    {
        np = n-1;
    }

    buf[np] = '-';
    buf[n-1] = '\n';
}

inline void etl(const char* city_blocks,
                const char* city_locs,
                const char* geo_asns,
                const char* output)
{
    LOG_CONTEXT("etl blocks %s locs %s asns %s into file %s", 
                 city_blocks,
                 city_locs,
                 geo_asns,
                 output);

    BinaryFile file;
    bool ok = file.open(output);

    if (!ok)
    {
        FATAL_ERROR("could not open %s for writing", output);
    }

    char buf[32]; get_header(buf, sizeof(buf));

    file.save_bytes_raw(buf, sizeof(buf));
    build_geo_data(file, city_blocks, city_locs, geo_asns);
}

#endif
