/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This class is used for interning strings. It uses a hash map to track the 
 * string to id mapping. The layout of indices and the char vector makes it 
 * easier to serialize later.
*/

#ifndef STRING_TABLE_HPP_A3ADA5DC
#define STRING_TABLE_HPP_A3ADA5DC

#include "macros.hpp"
#include "hash_map.hpp"

class StringTable
{
  public:
    StringTable() {}
    ~StringTable() {}

    size_t size() const
    {
        return indices_.size();
    }

    size_t byte_size() const
    {
        return strings_.size();
    }

    void insert(const std::string &s)
    {
        if (string_to_id_.count(s))
        {
            return;
        }

        unsigned index = indices_.size();
        string_to_id_[s] = index;

        indices_.push_back(strings_.size());

        strings_.insert(strings_.end(), s.begin(), s.end());
        strings_.push_back('\0');
    }

    unsigned index_of(const std::string &s) const
    {
        hash_map<std::string, unsigned>::const_iterator iter = 
            string_to_id_.find(s);

        if (iter == string_to_id_.end())
        {
            return 0xFFFFFFFF;
        }

        return iter->second;
    }

    const char* operator[](size_t i) const
    {
        return &strings_[indices_[i]];
    }

    const std::vector<unsigned> &indices() const { return indices_; }
    const std::vector<char> &strings() const { return strings_; }

  private:

    // default copy/assign would work in this class, but I am making
    // it an error because it is unexpected.

    DISALLOW_COPY_AND_ASSIGN(StringTable);

    hash_map<std::string, unsigned> string_to_id_;

    std::vector<unsigned> indices_;
    std::vector<char> strings_;
};

inline void save_string_table(BinaryFile &bf, const StringTable &st)
{
    bf.save_pod_vector(st.indices());
    bf.save_pod_vector(st.strings());
}

#endif
