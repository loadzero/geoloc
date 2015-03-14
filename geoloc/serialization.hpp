/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module contains classes for saving data into binary files, and loading 
 * it back in from memory maps.
*/

#ifndef SERIALIZATION_HPP_E69107D1
#define SERIALIZATION_HPP_E69107D1

#include "macros.hpp"
#include "error.hpp"

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

template <typename T>
struct RawMappedVector
{
    unsigned size_;
    T data_[];
};

template <typename T>
class MappedVector
{
  public:
    MappedVector()
        :
        ptr_(0)
    {
    }

    ~MappedVector()
    {
    }

    void init(const RawMappedVector<T>* ptr)
    {
        ptr_ = ptr;
    }

    size_t size() const
    {
        return ptr_->size_;
    }

    const T& operator[](size_t n) const
    {
        return ptr_->data_[n];
    }

    const T* begin() const
    {
        return ptr_->data_;
    }

    const T* end() const
    {
        return begin() + size();
    }

    size_t byte_size() const
    {
        return sizeof(ptr_->size_) + ptr_->size_ * sizeof(T);
    }

    // default copy/assign ok

  private:
    const RawMappedVector<T>* ptr_;
};

class MemoryMap
{
  public:
    MemoryMap()
        :
        data_(0),
        len_(0)
    {
    }

    ~MemoryMap()
    {
        if (data_)
        {
            int rc = munmap((void*) data_, len_);
            UNUSED(rc);
        }
    }

    bool open(const char* fn)
    {
        int fd = ::open(fn, O_RDONLY);

        if (fd < 0)
        {
            return false;
        }

        off_t end = lseek(fd, 0, SEEK_END);

        if (end == -1)
        {
            LOG_CONTEXT("could not seek to end of %s", fn);
            return false;
        }

        off_t beg = lseek(fd, 0, SEEK_SET);

        if (beg == -1)
        {
            LOG_CONTEXT("could not seek to beg of %s", fn);
            return false;
        }

        len_ = end;
        data_ = mmap(0, len_, PROT_READ, MAP_SHARED, fd, 0);

        if (!data_)
        {
            LOG_CONTEXT("could not mmap %s", fn);
            close(fd);
            return false;
        }

        int rc = ::close(fd);
        UNUSED(rc);

        return true;
    }

    const void* data() const
    {
        return data_;
    }

    const char* begin() const
    {
        return (const char*) data_;
    }

    size_t size() const
    {
        return len_;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(MemoryMap);

    const void *data_;
    size_t len_;
};

class BinaryFile
{
  public:
    BinaryFile()
        :
        file_(0)
    {
    }

    ~BinaryFile()
    {
        if (file_)
        {
            fclose(file_);
        }
    }

    bool open(const char* fn)
    {
        file_ = fopen(fn, "w");
        return file_;
    }

    off_t offset()
    {
        return ftello(file_);
    }

    void save_type(const char* x)
    {
        assert(strlen(x) == 4);
        size_t rn = fwrite(x, 1, 4, file_);

        if (rn != 4)
        {
            FATAL_ERROR("failed to save_type 4 bytes");
        }
    }

    void save_unsigned(unsigned x)
    {
        size_t rn = fwrite(&x, 1, 4, file_);

        if (rn != 4)
        {
            FATAL_ERROR("failed to save_unsigned 4 bytes");
        }
    }

    void save_bytes_padded(const void* b, size_t n)
    {
        size_t rn = fwrite(b, 1, n, file_);

        if (rn != n)
        {
            FATAL_ERROR("failed to save_bytes_padded %zu bytes", n);
        }

        return pad();
    }

    void save_bytes_raw(const void* b, size_t n)
    {
        size_t rn = fwrite(b, 1, n, file_);

        if (rn != n)
        {
            FATAL_ERROR("failed to save_bytes_raw %zu bytes", n);
        }
    }

    void seek(off_t where)
    {
        int rc = fseeko(file_, where, SEEK_SET);

        if (rc != 0)
        {
            FATAL_ERROR("failed to seek to %zu rc %d", where, rc);
        }
    }

    void pad()
    {
        // given the current offset, emit some padding bytes.

        unsigned padded = ((unsigned) (offset()) + 3) & ~0x3U;
        unsigned pad_bytes = padded - offset();
        char padding[] = {0,0,0,0};

        return save_bytes_raw(padding, pad_bytes);
    }

    template <typename T>
    void save_pod_vector(const std::vector<T> &v)
    {
        save_type("PODV");
        save_unsigned(0);

        off_t top = offset();
        save_unsigned(v.size());
        save_bytes_padded(&v[0], v.size() * sizeof(T));

        off_t bottom = offset();
        unsigned size = (bottom - top);

        seek(top-4);
        save_unsigned(size);
        seek(bottom);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(BinaryFile);

    FILE* file_;
    std::vector<off_t> offsets_;
};

inline bool isaligned(void* ptr)
{
    uintptr_t x = (uintptr_t)(ptr);
    return x % 4 == 0;
}

class MappedStringVector
{
  public:
    MappedStringVector()
        :
        data_(0),
        indices_()
    {
    }

    size_t size() const
    {
        return indices_->size_;
    }

    const char* operator[](size_t i) const
    {
        return data_ + indices_->data_[i];
    }

    void init(const char* data, const RawMappedVector<unsigned> *indices)
    {
        data_ = data;
        indices_ = indices;
    }

    // default copy/assign ok

  private:    
    const char* data_;
    const RawMappedVector<unsigned> *indices_;
};

class MemoryFile
{
  public:
    MemoryFile()
        :
        file_(),
        offset_(0)
    {
    }

    size_t avail()
    {
        return file_.size() - offset_;
    }

    bool open(const char* fn)
    {
        return file_.open(fn);
    }

    void* iter()
    {
        return (void*) (file_.begin() + offset_);
    }

    const void* get_mem(size_t n)
    {
        if (n > avail())
        {
            return 0;
        }

        void* out = iter();
        offset_ += n;

        return out;
    }

    const char* load_type()
    {
        return (const char*) get_mem(4);
    }

    const unsigned* load_unsigned()
    {
        REL_ASSERT(isaligned(iter()));
        return (const unsigned*)(get_mem(4));
    }

    template <typename T>
    const RawMappedVector<T>* load_raw_mapped_vector()
    {
        const char* type = load_type();

        if (type == 0)
        {
            return 0;
        }

        if (memcmp(type, "PODV", 4) != 0)
        {
            return 0;
        }

        const unsigned* len = load_unsigned();

        if (len == 0)
        {
            return 0;
        }

        return (const RawMappedVector<T>*)(get_mem(*len));
    }

    template <typename T>
    void load_mapped_vector(MappedVector<T> &out)
    {
        const RawMappedVector<T>* foo = load_raw_mapped_vector<T>();

        if (!foo)
        {
            FATAL_ERROR("could not load_mapped_vector");
        }

        out.init(foo);
    }

    void load_mapped_string_vector(MappedStringVector &out)
    {
        const RawMappedVector<unsigned>* foo = 
            load_raw_mapped_vector<unsigned>();

        if (!foo)
        {
            FATAL_ERROR("could not load_mapped_string_vector indices");
        }

        const RawMappedVector<char>* str = load_raw_mapped_vector<char>();

        if (!str)
        {
            FATAL_ERROR("could not load_mapped_string_vector strings");
        }

        out.init(str->data_, foo);
    }

  private:

    DISALLOW_COPY_AND_ASSIGN(MemoryFile);

    MemoryMap file_;
    off_t offset_;
};

#endif
