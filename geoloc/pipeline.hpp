/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module contains some pipeline framework utility classes. They are used 
 * to input data into the pipelines. Analogous to cat or echo.
*/

#ifndef PIPELINE_HPP_0D24961E
#define PIPELINE_HPP_0D24961E

#include <vector>
#include <string>

#include "connector.hpp"

class FileReader : public Connector
{
  public:
    explicit FileReader(const std::string &fn)
        :
        buffer_(0),
        cap_(0),
        file_(0)
    {
        if (fn == "-")
        {
            file_ = stdin;
        }
        else
        {
            file_ = fopen(fn.c_str(), "r");
        }

        if (!file_)
        {
            FATAL_ERROR("could not open %s", fn.c_str());
        }

        if (file_ == 0)
        {
            fprintf(stderr, "file %s dne\n", fn.c_str());
            exit(1);
        }
    }

    virtual ~FileReader()
    {
        if (file_ && file_ != stdin)
        {
            fclose(file_);
        }

        if (buffer_)
        {
            free(buffer_);
        }
    }
    
    void consume(const Buffer &b) {}

    void produce()
    {
        while (produce_one());
        flush();
    }

    bool produce_one()
    {
        ssize_t n = getline(&buffer_, &cap_, file_);

        if (n <= 0)
        {
            return false;
        }

        if (buffer_[n-1] == '\n')
        {
            buffer_[n-1] = '\0';
            n--;
        }

        emit(Buffer(buffer_, n));

        return true;
    }

  private:
    char* buffer_;
    size_t cap_;
    FILE* file_;
};

template <typename T>
class Collector : public Connector
{
  public:    
    explicit Collector(std::vector<T> &out)
        :
        out_(out)
    {
    }

    void consume(const Buffer &b)
    {
        const T* item = (const T*)(b.data());
        out_.push_back(*item);
    }

  private:
    std::vector<T> &out_;
};

class StringInjector : public Connector
{
  public:
    explicit StringInjector(const std::vector<std::string> &strings)
        :
        strings_(strings),
        index_(0)
    {
    }

    void consume(const Buffer &b) {}

    void produce()
    {
        while (produce_one());
        flush();
    }

    bool produce_one()
    {
        if (index_ == strings_.size())
        {
            return false;
        }

        const std::string& str = strings_[index_];

        emit(Buffer(str.c_str(), str.size()));
        index_++;

        return true;
    }

  private:

    const std::vector<std::string> &strings_;
    size_t index_;
};

#endif
