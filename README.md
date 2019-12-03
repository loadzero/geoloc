geoloc is a command line tool for bulk geolocation queries written in C++. 
Once its binary database has been built, ```geoloc``` performs geolocation 
queries offline.

Examples:

Bulk lookup from apache `access.log`:

```
$ cat access.log | awk '{print $1}' | geoloc -f - | column -t

10.172.47.117  AU  02  Sydney         -33.8001  151.3123   AS1610581   BIGCableCo
10.36.87.70    AU  07  Melbourne      -37.8266  144.7834   AS1370775   Micronode+PTY+LTD
10.88.81.165   US  CA  San+Francisco  37.6777   -122.2221  AS49335653  Big+Flare,+Inc
```

Query some IPs:

```
$ geoloc -q 8.8.8.8 192.30.252.131 --headers | column -t

ip              country  region  city           latitude  longitude  as_num   as_text
8.8.8.8         US       CA      Mountain+View  37.3860   -122.0838  AS15169  Google+Inc.
192.30.252.131  US       CA      San+Francisco  37.7697   -122.3933  AS36459  GitHub,+Inc.
```

```geoloc``` is designed to run fast and load fast:

```
$ wc -l /tmp/ip_list

 1000000 /tmp/ip_list

$ time geoloc -f /tmp/ip_list > /tmp/res1

real    0m6.131s
user    0m5.662s
sys     0m0.369s

$ time geoloc -q 8.8.8.8 192.30.252.131 > /tmp/res2

real    0m0.010s
user    0m0.002s
sys     0m0.005s

```

Installation
============

The program is designed as a portable application, to run out of ```~/bin```, 
with the database stored in ```~/var/db/geoloc/geodata.bin```.

To install:

```
$ git clone https://github.com/loadzero/geoloc.git && cd geoloc
$ ./configure
$ make
$ make install
```

The configure script will check for these dependencies:

- iconv
- unzip
- wget
- make
- c++

During installation, data will be downloaded from 
[MaxMind](http://dev.maxmind.com/geoip/legacy/geolite/) to create the database.

An update script will be installed into ```~/bin/_geoloc_update.sh```. Run 
this script when you would like to update your geolocation database. MaxMind 
updates their source data once a month.

I have tested on OSX 10.9.5 and Ubuntu 14.04. Other unices are likely to work 
with minimal or no changes. It is unlikely to work on windows, due to the use 
of [mmap](http://en.wikipedia.org/wiki/Mmap).

Design and Implementation
=========================

The code operates in two phases, packing and query. The packing phase is all
about converting the data into a machine optimal format, namely relocatable
sorted vectors. The query phase simply mmaps that data, and performs a
std::upper\_bound binary search on it to find the IPs.

There is an outline of the code, roughly in topological order 
[here](outline.md), that contains a summary of each module.

Attribution
===========

This software includes GeoLite data created by MaxMind available from 
[http://www.maxmind.com](http://www.maxmind.com)
