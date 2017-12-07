/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-07
 *
 * This file contains test code for geoloc. It is mostly serialization tests.
*/

#include "serialization.hpp"
#include "string_table.hpp"

#include <string.h>
#include <stdarg.h>

struct Poddable
{
    int a;
    int b;
    int c;

    bool operator==(const Poddable &o) const
    {
        return memcmp(this, &o, sizeof(o)) == 0;
    }
};

static int test_poddable_roundtrip();
static int test_string_table_roundtrip();

int main(int argc, char** argv)
{
    // serialization tests

    test_poddable_roundtrip();
    test_string_table_roundtrip();
}

static int test_poddable_roundtrip()
{
    std::vector<Poddable> bar_vec;

    {
        for (size_t i = 0; i < 100; ++i)
        {
            Poddable bar;
            bar.a = 100 + i;
            bar.b = 1000 + i;
            bar.c = 10000 + i;

            bar_vec.push_back(bar);
        }

        BinaryFile bf;
        
        bf.open("tmp/foo.bin");
        bf.save_pod_vector(bar_vec);
    }

    MemoryFile mf;
    mf.open("tmp/foo.bin");

    MappedVector<Poddable> foo;
    mf.load_mapped_vector(foo);

    assert(foo.size() != 0);
    assert(foo.size() == bar_vec.size());

    for (size_t i = 0; i < bar_vec.size(); ++i)
    {
        assert(foo[i] == bar_vec[i]);
    }

    return 0;
}

static int test_string_table_roundtrip()
{
    {
        StringTable st;

        st.insert("aaaa");
        st.insert("aaaa");
        st.insert("aaaa");

        assert(st.size() == 1);
        assert(st.byte_size() == 5);
        assert(st.index_of("aaaa") == 0);

        assert(strcmp(st[0], "aaaa") == 0);

        st.insert("bbb");

        assert(st.size() == 2);
        assert(st.byte_size() == 9);

        assert(st.index_of("bbb") == 1);
        assert(strcmp(st[1], "bbb") == 0);

        BinaryFile bf;
        bf.open("tmp/sv.bin");

        for (size_t i = 0; i < 1000; ++i)
        {
            char buf3[1024];

            sprintf(buf3, "funky str %d %d %d\n", (int) i, (int) i * 1234,
                    (int) i * 123456);
            st.insert(buf3);
        }

        save_string_table(bf, st);
    }

    MemoryFile mf;
    mf.open("tmp/sv.bin");

    MappedStringVector mv;
    mf.load_mapped_string_vector(mv);

    assert(mv.size() == 1002);

    assert(strcmp(mv[0], "aaaa") == 0);
    assert(strcmp(mv[1], "bbb") == 0);

    for (size_t i = 0; i < 1000; ++i)
    {
        char buf3[1024];
        sprintf(buf3, "funky str %d %d %d\n", (int) i, (int) i * 1234,
                (int) i * 123456);
        assert(strcmp(mv[i+2], buf3) == 0);
    }

    return 0;
}
