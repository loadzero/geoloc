#!/bin/bash

set -e
set -u

# geoloc database updater script
# uses wget to download relevant csv from maxmind and transforms to UTF-8
# finally it builds the db with geoloc --import

tok=$(dd if=/dev/urandom bs=12 count=1 2>/dev/null | xxd -p)

mkdir -p /tmp/geoloc.$tok
echo "staging area /tmp/geoloc.$tok"

cd /tmp/geoloc.$tok

echo "downloading from loadzero"
echo "warning: This geoip database is old (from 2017)"
echo "warning: For experimental use only"

curl -L -O http://blog.loadzero.com/assets/geodata.bin.gz
gunzip geodata.bin.gz
mkdir -p ~/var/db/geoloc
mv geodata.bin ~/var/db/geoloc
rm -rf "/tmp/geoloc.$tok"

# Deprecated as of Jan 2019
#
#wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity_CSV/GeoLiteCity-latest.zip
#wget http://download.maxmind.com/download/geoip/database/asnum/GeoIPASNum2.zip
#
#blocks_file=$(unzip -l GeoLiteCity-latest.zip |grep City.Blocks.csv | awk '{print $4}')
#dir=$(dirname $blocks_file)
#frag=""
#
#if ! echo $dir | grep "^GeoLiteCity_........$"; then
#    echo error zip file "$blocks_file" looks weird
#    exit 1
#fi
#
#unzip GeoLiteCity-latest.zip
#unzip GeoIPASNum2.zip
#
#iconv -f ISO-8859-15 -t UTF-8 $dir/GeoLiteCity-Blocks.csv > blocks.csv
#iconv -f ISO-8859-15 -t UTF-8 $dir/GeoLiteCity-Location.csv > location.csv
#iconv -f ISO-8859-15 -t UTF-8 GeoIPASNum2.csv > asnum.csv
#
#mkdir -p ~/var/db/geoloc
#
## create a new db file
#~/bin/geoloc --import /tmp/geoloc.$tok -o data.bin
#
## swap into place atomically only on success
#
#mv data.bin ~/var/db/geoloc/geodata.bin
#
#rm -rf "/tmp/geoloc.$tok"
