#!/bin/bash
set -ex

# setup folder-structure in volume
mkdir -p /data/Config
mkdir -p /data/Licenses
mkdir -p /data/Logging
mkdir -p /data/Nodelocks
mkdir -p /model

# create the expected folder structure
ln -s /data/Config /usr/local/Aimms/Config
ln -s /data/Licenses /usr/local/Aimms/Licenses
ln -s /data/Logging /usr/local/Aimms/Logging
ln -s /data/Nodelocks /usr/local/Aimms/Nodelocks

if [ "$1" = 'jobrunner' ]; then
    cd /model
    shift
    exec /usr/local/Aimms/Bin/jobrunner "$@"
else
    exec "$@"
fi
