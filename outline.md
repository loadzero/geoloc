geoloc/args.hpp
--------------------------

This file contains a command line argument helper class.

geoloc/error.hpp
--------------------------

This file contains the declarations needed to support logging and error handling.

geoloc/error.cpp
--------------------------

This file contains the logging and error handling implementation. We use a mirrored ring buffer to handle log messages. The ring buffer is dumped out to stderr when an assert or fatal error fires.

Note - log messages larger than 4095 bytes will get truncated to 4095 bytes.

geoloc/csv.hpp
--------------------------

This file contains utility functions for tokenizing and parsing strings.

geoloc/macros.hpp
--------------------------

This file contains macros that are commonly used throughout the source.

geoloc/hash\_map.hpp
--------------------------

This file contains macros to paper over differences between older/newer compilers as far as using std::unordered_map is concerned.

geoloc/connector.hpp
--------------------------

This module contains the pipeline framework core classes.

The basic idea is similar to a unix pipeline, allowing the user to connect Connectors like so:

a | b | c

A Connector is analogous to a unix filter, and a Buffer is analogous to a line of text.

geoloc/string\_table.hpp
--------------------------

This class is used for interning strings. It uses a hash map to track the string to id mapping.
The layout of indices and the char vector makes it easier to serialize later.

geoloc/serialization.hpp
--------------------------

This module contains classes for saving data into binary files, and loading it back in from memory maps.

geoloc/pipeline.hpp
--------------------------

This module contains some pipeline framework utility classes. They are used to input data into the pipelines. Analogous to cat or echo.

geoloc/locations.hpp
--------------------------

This module handles representations of location data, both normal and packed formats.

A Location stores an id and positional information from the MaxMind dataset, namely:

country, region, city, latitude, longitude

geoloc/asns.hpp
--------------------------

This module handles representations of ASN data, both normal and packed formats.

An ASN describes an ip range, [autonomous system number](http://en.wikipedia.org/wiki/Autonomous_system_%28Internet%29), and some text describing the system.

geoloc/blocks.hpp
--------------------------

This module handles representations of IP ranges, both normal and packed formats.

A Block is an ip range, and an index into another structure.

geoloc/etl.hpp
--------------------------

This module contains helper functions to extract, transform and load a MaxMind csv dataset.

geoloc/query.hpp
--------------------------

This module handles the query phase of geoloc. It is reponsible for loading the geodata file, and providing interfaces for querying it. 

The main part of the query code uses a binary search (std::upper\_bound) against a set of memory mapped sorted vectors.

geoloc/geoloc.cpp
--------------------------

```geoloc``` is a command line application for retrieving MaxMind geolocation and ASN info for a set of IP addresses.

The default output format is one record per line, space separated, with no headers. The columns output are as follows:

ip, country, region, city, latitude, longitude, as_num, as\_text

geoloc/test.cpp
--------------------------

This file contains test code for geoloc. It is mostly serialization tests.

