/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-06
 *
 * geoloc is a command line application for retrieving MaxMind geolocation and 
 * ASN info for a set of IP addresses.
 * 
 * The default output format is one record per line, space separated, with no 
 * headers. The columns output are as follows:
 * 
 * ip, country, region, city, latitude, longitude, as_num, as_text
*/

#include <stdio.h>
#include <stdlib.h>

#include "etl.hpp"
#include "query.hpp"
#include "error.hpp"
#include "args.hpp"

#include <set>

static void usage(const char* condition)
{
    if (condition)
    {
        fprintf(stderr, "%s\n", condition);
    }

    fprintf(stderr, "usage:");
    fprintf(stderr, "\tgeoloc -f file ... [--headers]\n");
    fprintf(stderr, "\tgeoloc -q ip ...\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "This software includes GeoLite data created by MaxMind\n");
    fprintf(stderr, "available from http://www.maxmind.com\n");

    exit(1);
}

static std::string default_file()
{
    const char* home_env = getenv("HOME");
    std::string home_dir = home_env ? home_env : "";

    if (home_dir.empty())
    {
        FATAL_ERROR("could not get home dir");
    }

    return home_dir + "/var/db/geoloc/geodata.bin";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        usage(0);
    }

    Args args(argc, argv);

    std::set<std::string> flags;

    flags.insert("-f");
    flags.insert("--import");
    flags.insert("--headers");
    flags.insert("-q");
    flags.insert("-o");

    std::vector<std::string> input_list;
    std::string import;
    std::string output;

    std::string data_file_name = default_file();
    bool show_headers = false;

    while (!args.empty())
    {
        if (strcmp(args.peek(), "-f") == 0)
        {
            args.pop();

            while (true)
            {
                const char* fn = args.pop();

                if (!fn)
                {
                    usage("empty file arg");
                }

                input_list.push_back(std::string("file:") + fn);

                if (args.empty() || flags.count(args.peek()))
                    break;
            }
        }
        else if (strcmp(args.peek(), "--headers") == 0)
        {
            show_headers = true;
            args.pop();
        }
        else if (strcmp(args.peek(), "-o") == 0)
        {
            args.pop();

            const char* arg = args.pop();

            if (!arg)
            {
                usage("empty output arg");
            }

            output = arg;
        }
        else if (strcmp(args.peek(), "--import") == 0)
        {
            args.pop();

            if (!import.empty())
            {
                usage("too many import options");
            }

            const char* arg = args.pop();

            if (!arg)
            {
                usage("empty import arg");
            }

            import = arg;
        }
        else if (strcmp(args.peek(), "-q") == 0)
        {
            args.pop();

            std::string query = "query:";

            while (true)
            {
                const char* arg = args.pop();

                if (!arg)
                {
                    usage("empty query arg");
                }

                query += arg + std::string(",");

                if (args.empty() || flags.count(args.peek()))
                    break;
            }

            input_list.push_back(query);
        }
        else
        {
            fprintf(stderr, "unrecognized option %s\n", args.peek());
            usage(0);
        }
    }

    if (!import.empty())
    {
        if (!input_list.empty())
        {
            usage("import and query are mutually exclusive");
        }

        if (output.empty())
        {
            usage("no output specified with import");
        }

        std::string city_blocks = import + "/blocks.csv";
        std::string city_locs = import + "/location.csv";
        std::string geo_asns = import + "/asnum.csv";

        etl(city_blocks.c_str(), city_locs.c_str(), geo_asns.c_str(), 
            output.c_str());
    }
    else
    {
        if (input_list.empty())
        {
            usage("query has no input");
        }

        if (!import.empty())
        {
            usage("import and query are mutually exclusive");
        }

        query(data_file_name.c_str(), input_list, show_headers);
    }

    return 0;
}
