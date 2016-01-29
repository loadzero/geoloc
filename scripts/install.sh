#!/bin/bash

set -e
set -u

# geoloc installer script - responsible for installing binaries and scripts
# and ensuring that a database is created

# this script is meant to be called from make install

if [[ ! -e ~/bin ]]; then
    mkdir -p ~/bin
fi

cp bin/geoloc ~/bin
cp scripts/_geoloc_update.sh ~/bin

~/bin/_geoloc_update.sh

# do a self test

result=$(~/bin/geoloc -q 8.8.8.8)
expected="8.8.8.8 US CA Mountain+View 47.3845 -122.0881 AS15169 Google+Inc."

if [[ "$result" != "$expected" ]]; then
    echo "ERROR: unexpected self-test result"
    exit 1
fi
