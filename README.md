geoloc
==================

```geoloc``` is a command line tool for bulk lookup of geolocation data for IPv4 addresses.

Once its binary database has been built, ```geoloc``` performs geolocation queries offline.

Examples:

Bulk lookup from apache access.log:

    $ cat access.log | awk '{print $1}' | geoloc -f - | column -t

    10.172.47.117  AU  02  Sydney         -33.8001  151.3123   AS1610581   BIGCableCo
    10.36.87.70    AU  07  Melbourne      -37.8266  144.7834   AS1370775   Micronode+PTY+LTD
    10.88.81.165   US  CA  San+Francisco  37.6777   -122.2221  AS49335653  Big+Flare,+Inc

Query some IPs:

    $ geoloc -q 8.8.8.8 192.30.252.131 --headers | column -t

    ip              country  region  city           latitude  longitude  as_num   as_text
    8.8.8.8         US       CA      Mountain+View  37.3860   -122.0838  AS15169  Google+Inc.
    192.30.252.131  US       CA      San+Francisco  37.7697   -122.3933  AS36459  GitHub,+Inc.

```geoloc``` is designed to be fast:

    $ wc -l /tmp/ip_list

     1000000 /tmp/ip_list

    $ time bin/geoloc -f /tmp/ip_list > /tmp/res

    real	0m6.131s
    user	0m5.662s
    sys     0m0.369s

Installation
------------

The program is designed as a portable application, to run out of ```~/bin```, with the database stored in ```~/var/db/geoloc/geodata.bin```

To install:

    $ git clone https://github.com/loadzero/geoloc.git
    $ ./configure
    $ make
    $ make install

For Developers
--------------

Coming soon.

In the meantime, check out the [outline](outline.md).

Attribution
-----------

This software includes GeoLite data created by MaxMind available from [http://www.maxmind.com](http://www.maxmind.com)
