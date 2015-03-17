/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module handles the query phase of geoloc. It is reponsible for loading 
 * the geodata file, and providing interfaces for querying it. 
 * 
 * The main part of the query code uses a binary search (std::upper_bound) 
 * against a set of memory mapped sorted vectors.
*/

#ifndef QUERY_HPP_EC6CE5A7
#define QUERY_HPP_EC6CE5A7

#include "macros.hpp"
#include "error.hpp"
#include "blocks.hpp"
#include "locations.hpp"
#include "asns.hpp"
#include "csv.hpp"
#include "pipeline.hpp"

#include <algorithm>

struct IPResult
{
    IPResult()
        :
        quad(0),
        country(0),
        region(0),
        city(0),
        lat(0),
        lon(0),
        asn(0),
        asn_text(0)
    {
    }

    unsigned quad;

    const char* country;
    const char* region;
    const char* city;

    float lat;
    float lon;

    const unsigned* asn;
    const char* asn_text;
};

class GeoData
{
  public:
    GeoData() {}
    ~GeoData() {}

    void open(const char* fn)
    {
        LOG_CONTEXT("GeoData open %s", fn);

        bool ok = mem_file_.open(fn);

        if (!ok)
        {
            FATAL_ERROR("could not open %s for reading", fn);
        }

        LOG_CONTEXT("GeoData read header");
        read_header();

        LOG_CONTEXT("GeoData load location_ip_blocks");
        location_ip_blocks_.load(mem_file_);

        LOG_CONTEXT("GeoData load location_data");
        location_data_.load(mem_file_);

        LOG_CONTEXT("GeoData load asn_ip_blocks");
        asn_ip_blocks_.load(mem_file_);

        LOG_CONTEXT("GeoData load asn_data");
        asn_data_.load(mem_file_);
    }

    void check_header_value(const char* type,
                            const char* value,
                            const char* expected)
    {
        if (strcmp(value, expected) != 0)
        {
            FATAL_ERROR("header %s expecting %s got %s",
                        type, value, expected);
        }
    }

    void read_header()
    {
        const char* raw = (const char*) mem_file_.get_mem(32);

        std::string header(raw, raw + 32);
        std::string scratch;
        std::vector<char*> toks;

        char_split(header, scratch, toks, ' ');

        if (toks.size() != 5)
        {
            FATAL_ERROR("header is corrupt - %s", header.c_str());
        }

        check_header_value("header1", toks[0], "geoloc");
        check_header_value("header2", toks[1], "loadzero");
        check_header_value("version", toks[2], "v001");
        check_header_value("endian", toks[3], get_endian());
    }

    unsigned block_query(const BlockTable &blocks, unsigned quad) const
    {
        // find first pos compares gt quad
        
        const unsigned* iter = 
            std::upper_bound(blocks.start_ip.begin(),
                             blocks.start_ip.end(),
                             quad);

        if (iter == blocks.start_ip.begin())
        {
            return -1;
        }

        unsigned idx = iter - blocks.start_ip.begin();
        unsigned ri = idx - 1;

        if (quad >= blocks.start_ip[ri] &&
            quad <= blocks.end_ip[ri])
        {
            return ri;
        }

        return -1;
    }

    unsigned location_block_query(unsigned quad) const
    {
        return block_query(location_ip_blocks_, quad);
    }

    void query(unsigned quad, IPResult &result) const
    {
        result.quad = quad;

        unsigned block_idx = block_query(location_ip_blocks_, quad);

        if (block_idx != -1U)
        {
            unsigned loc_idx = location_ip_blocks_.loc[block_idx];
            const PackedLocation& loc = location_data_.locations[loc_idx];

            result.country = location_data_.country[loc.country];
            result.region = location_data_.region[loc.region];
            result.city = location_data_.city[loc.city];
            result.lat = loc.lat;
            result.lon = loc.lon;
        }

        block_idx = block_query(asn_ip_blocks_, quad);

        if (block_idx != -1U)
        {
            unsigned asn_idx = asn_ip_blocks_.loc[block_idx];
            const PackedASN& asn = asn_data_.asns[asn_idx];

            result.asn = &asn.number;
            result.asn_text = asn_data_.text[asn.text];
        }
    }

  private:

    DISALLOW_COPY_AND_ASSIGN(GeoData);

    MemoryFile mem_file_;

    BlockTable location_ip_blocks_;
    LocationTable location_data_;

    BlockTable asn_ip_blocks_;
    ASNTable asn_data_;
};

inline int ip_to_s(char* out, unsigned quad)
{
    int a = (quad >> 24) & 0xff;
    int b = (quad >> 16) & 0xff;
    int c = (quad >> 8) & 0xff;
    int d = (quad) & 0xff;

    return sprintf(out, "%d.%d.%d.%d", a, b, c, d);
}

// convert dotted quads into unsigned ints
class IPParser : public Connector
{
  public:
    void consume(const Buffer &b)
    {
        str_.assign((char*) b.data(), b.size());
        char_split(str_, scratch_, toks, '.');

        if (toks.size() != 4)
        {
            return;
        }

        unsigned quad = to_u(toks[0]) << 24 | 
                        to_u(toks[1]) << 16 | 
                        to_u(toks[2]) << 8 | 
                        to_u(toks[3]);

        emit(Buffer(&quad, sizeof(quad)));
    }

  private:
    std::string str_;
    std::string scratch_;
    std::vector<char*> toks;
};

class IPScanner: public Connector
{
  public:
    explicit IPScanner(const GeoData &geo_data)
        :
        geo_data_(geo_data)
    {
    }

    void consume(const Buffer &b)
    {
        unsigned quad = *(unsigned*)(b.data());

        IPResult result;
        geo_data_.query(quad, result);

        emit(Buffer(&result, sizeof(result)));
    }

  private:
    const GeoData &geo_data_;
};

// currently just turns spaces into +
// TODO - escape into percent encoded ASCII.
inline void escape(std::string &out, const char* str)
{
    if (!str)
    {
        out = "%";
        return;
    }

    size_t n = strlen(str);

    if (n == 0)
    {
        out = "%";
        return;
    }

    out.resize(n);

    char* out_iter = &out[0];
    const char* in_iter = str;

    while (*in_iter)
    {
        int outc = (*in_iter == ' ') ? '+' : *in_iter;
        *out_iter++ = outc;

        ++in_iter;
    }
}

class IPResultEmitter : public Connector
{
  public:
    void print_ip(unsigned quad)
    {
        char buf[32];
        int nb = ip_to_s(buf, quad);

        writes(buf, nb);
    }

    void print(const char* s)
    {
        escape(esc_buf_, s);

        writes(esc_buf_.c_str(), esc_buf_.size());
    }

    void print(float f) 
    {
        char fbuf[32];
        int nb = sprintf(fbuf, "%3.4f", f);

        writes(fbuf, nb);
    }

    void print_as(const unsigned *asn) 
    {
        char sbuf[32];
        int nb = 0;

        if (!asn)
        {
            nb = sprintf(sbuf, "%%");
        }
        else
        {
            nb = sprintf(sbuf, "AS%u", *asn);
        }

        writes(sbuf, nb);
    }

    static void show_headers()
    {
        fprintf(stdout, 
                "ip country region city latitude longitude as_num as_text\n");
    }

    void delimit() 
    {
        writes(" ", 1);
    }

    void newline()
    {
        writes("\n", 1);
    }

    void writes(const char* s, size_t n)
    {
        print_buf_.insert(print_buf_.end(), s, s+n);
    }

    void consume(const Buffer &b)
    {
        const IPResult* result = (const IPResult*)(b.data());

        print_buf_.clear();

        print_ip(result->quad); delimit();
        print(result->country); delimit();
        print(result->region); delimit();
        print(result->city); delimit();
        print(result->lat); delimit();
        print(result->lon); delimit();
        print_as(result->asn); delimit();
        print(result->asn_text);
        newline();

        size_t n = fwrite(&print_buf_[0], print_buf_.size(), 1, stdout);
        REL_ASSERT(n == 1);

        print_buf_.clear();
    }

  private:

    std::string print_buf_;
    std::string esc_buf_;
};

template <typename T>
inline void query(T &reader, GeoData &data)
{
    IPParser parser;
    IPScanner scanner(data);

    IPResultEmitter emitter;

    reader | parser | scanner | emitter;
    reader.produce();
}

inline void query(GeoData &data, const std::string &source)
{
    LOG_CONTEXT("query data with source %s", source.c_str());
    
    const char* iter = &source[0];
    const char* end = &source[0] + source.size();
    const char* res = strchr(iter, ':');

    if (!res)
    {
        FATAL_ERROR("could not parse source %s", source.c_str());
    }

    std::string protocol(iter, res);
    std::string path(res+1, end);

    if (protocol == "file")
    {
        FileReader reader(path);
        query(reader, data);
    }
    else if (protocol == "query")
    {
        std::vector<char*> toks;
        std::string scratch;
        csv_split(&path[0], path.size(), scratch, toks);

        std::vector<std::string> ip_list;
        ip_list.assign(toks.begin(), toks.end());

        StringInjector reader(ip_list);
        query(reader, data);
    }
    else
    {
        FATAL_ERROR("unknown source protocol %s", protocol.c_str());
    }
}

inline void query(const char* data_file_name,
                  const std::vector<std::string> &data_sources,
                  bool show_headers)
{
    LOG_CONTEXT("query data %s with %zu sources", data_file_name, data_sources.size());

    GeoData data;
    data.open(data_file_name);

    if (show_headers)
    {
        IPResultEmitter::show_headers();
    }

    for (size_t i = 0; i < data_sources.size(); ++i)
    {
        query(data, data_sources[i]);
    }
}

#endif
