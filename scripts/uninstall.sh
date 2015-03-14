#!/bin/bash

set -e
set -u

if [[ -e ~/bin/geoloc ]]; then
    rm ~/bin/geoloc
fi

if [[ -e ~/bin/_geoloc_update.sh ]]; then
    rm ~/bin/_geoloc_update.sh
fi

if [[ -e ~/var/db/geoloc/geodata.bin ]]; then
    rm ~/var/db/geoloc/geodata.bin
fi
