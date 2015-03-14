/*
 * Copyright 2015 Jason McSweeney
 * Licensed under BSD 3 Clause - see LICENSE
 *
 * author: Jason McSweeney
 * created: 2015-03-11
 *
 * This module contains the pipeline framework core classes.
 * 
 * The basic idea is similar to a unix pipeline, allowing the user to connect 
 * Connectors like so:
 * 
 * a | b | c
 * 
 * A Connector is analogous to a unix filter, and a Buffer is analogous to a 
 * line of text.
*/

#ifndef CONNECTOR_HPP_576AAD9D
#define CONNECTOR_HPP_576AAD9D

#include "macros.hpp"

class Buffer
{
  public:
    Buffer()
        :
        data_(0),
        n_(0)
    {
    }

    Buffer(const void* p, size_t n)
    {
        data_ = p;
        n_ = n;
    }

    ~Buffer()
    {
    }

    const void* data() const
    {
        return data_;
    }

    size_t size() const
    {
        return n_;
    }

    // default copy/assign is fine.

  private:
    const void *data_;
    size_t n_;
};

class Connector
{
  public:
    Connector()
        :
        downstream_(0)
    {
    }

    virtual void emit(const Buffer &b)
    {
        if (!downstream_)
        {
            return;
        }

        downstream_->consume(b);
    }

    virtual void emit_flush()
    {
        if (!downstream_)
        {
            return;
        }

        downstream_->flush();
    }

    virtual void consume(const Buffer &b) = 0;

    virtual void flush()
    {
        emit_flush();
    }

    virtual Connector& operator|(Connector &c)
    {
        downstream_ = &c;
        return *downstream_;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Connector);

    Connector* downstream_;
};

#endif
