/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module handles representations of IP ranges, both normal and packed 
 * formats.
 * 
 * A Block is an ip range, and an index into another structure.
*/

#ifndef BLOCKS_HPP_57764690
#define BLOCKS_HPP_57764690

#include "csv.hpp"
#include "error.hpp"

struct Block
{
    unsigned start_ip;
    unsigned end_ip;
    unsigned loc;
};

class BlockTable
{
  public:
    void load(MemoryFile& file)
    {
        LOG_CONTEXT("BlockTable load");

        file.load_mapped_vector(start_ip);
        file.load_mapped_vector(end_ip);
        file.load_mapped_vector(loc);
    }

    MappedVector<unsigned> start_ip; 
    MappedVector<unsigned> end_ip; 
    MappedVector<unsigned> loc; 

    // default copy/assign is fine
};

class BlockParser : public Connector
{
  public:
    BlockParser()
        :
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

        if (toks_.size() != 3)
        {
            // we just silently drop bad lines
            return;
        }

        Block block;

        block.start_ip = to_u(toks_[0]);
        block.end_ip = to_u(toks_[1]);
        block.loc = to_u(toks_[2]);

        emit(Buffer(&block, sizeof(block)));
    }

    std::string scratch_;
    std::vector<char*> toks_;

    size_t line_;
};

inline bool save_blocks(BinaryFile &file, const std::vector<Block> &v)
{
    std::vector<unsigned> start_ip;
    std::vector<unsigned> end_ip;
    std::vector<unsigned> loc;

    start_ip.resize(v.size());
    end_ip.resize(v.size());
    loc.resize(v.size());

    unsigned last = 0;

    // split into columns and check sortedness

    for (size_t i = 0; i < v.size(); ++i)
    {
        assert(v[i].start_ip > last);
        assert(v[i].end_ip >= v[i].start_ip);

        start_ip[i] = v[i].start_ip;
        end_ip[i] = v[i].end_ip;
        loc[i] = v[i].loc;

        last = v[i].end_ip;
    }

    file.save_pod_vector(start_ip);
    file.save_pod_vector(end_ip);
    file.save_pod_vector(loc);

    return true;
}

#endif
