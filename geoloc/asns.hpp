/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module handles representations of ASN data, both normal and packed 
 * formats.
 * 
 * An ASN describes an ip range, autonomous system number, and some text 
 * describing the system.
*/

#ifndef ASNS_HPP_5623F7C7
#define ASNS_HPP_5623F7C7

#include "serialization.hpp"
#include "csv.hpp"
#include "blocks.hpp"
#include "hash_map.hpp"

struct ASN
{
    unsigned start_ip;
    unsigned end_ip;
    unsigned number;

    std::string text;
};

struct PackedASN
{
    unsigned number;
    unsigned text;
};

class ASNTable
{
  public:
    void load(MemoryFile& file)
    {
        file.load_mapped_string_vector(text);
        file.load_mapped_vector(asns);
    }

    MappedStringVector text;
    MappedVector<PackedASN> asns;
};

class ASNParser : public Connector
{
  public:
    ASNParser()
        :
        line_(0)
    {
    }

    void parse_text(char* str, unsigned &num, std::string &txt)
    {
        scratch_ = str;

        char* iter = &scratch_[0];
        char* end = iter + scratch_.size();

        char* sp = strchr(iter, ' ');

        const char* tok0 = iter;
        const char* tok1 = sp ? sp + 1 : end;

        REL_ASSERT(strlen(tok0) > 2);

        num = to_u(tok0 + 2);
        txt = tok1;
    }
    
    void consume(const Buffer &b)
    {
        ++line_;

        csv_split((const char*) b.data(), b.size(), scratch_, toks);

        if (toks.size() != 3)
        {
            return;
        }

        ASN asn;

        asn.start_ip = to_u(toks[0]);
        asn.end_ip = to_u(toks[1]);
        parse_text(toks[2], asn.number, asn.text);

        emit(Buffer(&asn, sizeof(asn)));
    }

  private:

    std::string str_;
    std::string scratch_;
    std::vector<char*> toks;

    size_t line_;
};

// split the asn data into two tables
// a block table and an info table
inline void save_asns(BinaryFile &file, const std::vector<ASN> &asns)
{
    hash_map<unsigned, unsigned> asn_to_idx;
    std::vector<PackedASN> packed_asns;

    StringTable text;

    for (size_t i = 0; i < asns.size(); ++i)
    {
        const ASN &asn = asns[i];

        if (asn_to_idx.count(asn.number))
        {
            continue;
        }

        asn_to_idx[asn.number] = packed_asns.size();
        text.insert(asn.text);

        PackedASN pasn;

        pasn.number = asn.number;
        pasn.text = text.index_of(asn.text);

        asn_to_idx[asn.number] = packed_asns.size();
        packed_asns.push_back(pasn);
    }

    std::vector<Block> asn_blocks(asns.size());

    unsigned last = 0;

    for (size_t i = 0; i < asns.size(); ++i)
    {
        Block &block = asn_blocks[i];
        const ASN &asn = asns[i];

        REL_ASSERT(asn.start_ip > last);
        REL_ASSERT(asn.end_ip >= asn.start_ip);

        block.start_ip = asn.start_ip;
        block.end_ip = asn.end_ip;
        block.loc = asn_to_idx[asn.number];

        last = asn.end_ip;
    }

    save_blocks(file, asn_blocks);
    save_string_table(file, text);
    file.save_pod_vector(packed_asns);
}

#endif
