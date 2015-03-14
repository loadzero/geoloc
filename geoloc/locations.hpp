/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module handles representations of location data, both normal and packed 
 * formats.
 * 
 * A Location stores an id and positional information from the MaxMind dataset, 
 * namely:
 * 
 * country, region, city, latitude, longitude
*/

#ifndef LOCATIONS_HPP_CCAC801C
#define LOCATIONS_HPP_CCAC801C

#include "string_table.hpp"
#include "error.hpp"
#include "csv.hpp"

struct Location
{
    Location()
        :
        id(0)
    {
    }

    unsigned id;
    std::string country;
    std::string region;
    std::string city;
    std::string lat;
    std::string lon;
};

struct PackedLocation
{
    PackedLocation()
        :
        id(0),
        country(0),
        region(0),
        city(0),
        lat(0.0),
        lon(0.0)
    {
    }

    unsigned id;
    unsigned country;
    unsigned region;
    unsigned city;

    float lat;
    float lon;
};

class LocationTable
{
  public:
    void load(MemoryFile& file)
    {
        file.load_mapped_string_vector(country);
        file.load_mapped_string_vector(region);
        file.load_mapped_string_vector(city);
        file.load_mapped_vector(locations);
    }

    void dump()
    {
        printf("loc size %d\n", (int) locations.size());

        for (size_t i = 0; i < locations.size(); ++i)
        {
            const PackedLocation &loc = locations[i];

            printf("%d %s %s %s\n", loc.id,
                   country[loc.country],
                   region[loc.region],
                   city[loc.city]);
        }
    }

    // default copy/assign ok

    MappedStringVector country;
    MappedStringVector region;
    MappedStringVector city;
    MappedVector<PackedLocation> locations;
};

class LocationParser : public Connector
{
  public:
    LocationParser()
        :
        str_(),
        scratch_(),
        toks_(),
        line_(0)
    {
    }
    
    void consume(const Buffer &b)
    {
        line_++;
        if (line_ < 3) return;

        csv_split((const char*) b.data(), b.size(), scratch_, toks_);

        if (toks_.size() != 9)
        {
            return;
        }

        Location loc;

        loc.id = to_u(toks_[0]);
        loc.country = toks_[1];
        loc.region = toks_[2];
        loc.city = toks_[3];
        loc.lat = toks_[5];
        loc.lon = toks_[6];

        emit(Buffer(&loc, sizeof(loc)));
    }

  private:
    std::string str_;
    std::string scratch_;
    std::vector<char*> toks_;

    size_t line_;
};

// convert string columns into string tables
// turn location into packed location
inline void save_locations(BinaryFile &file, 
                           const std::vector<Location> &locations)
{
    unsigned maxid = 0;

    StringTable country;
    StringTable region;
    StringTable city;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const Location &loc = locations[i];

        maxid = std::max(loc.id, maxid);

        country.insert(loc.country);
        region.insert(loc.region);
        city.insert(loc.city);
    }

    save_string_table(file, country);
    save_string_table(file, region);
    save_string_table(file, city);

    std::vector<PackedLocation> packed;
    packed.resize(maxid + 1);

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const Location &loc = locations[i];

        PackedLocation &ploc = packed[loc.id];

        ploc.id = loc.id;
        ploc.country = country.index_of(loc.country);
        ploc.region = region.index_of(loc.region);
        ploc.city = city.index_of(loc.city);
        ploc.lat = strtof(loc.lat.c_str(), 0);
        ploc.lon = strtof(loc.lon.c_str(), 0);
    }

    file.save_pod_vector(packed);
}

#endif
