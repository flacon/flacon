#!/bin/bash

set -e

ACTUAL=$(basename $1).actual
EXPECTED=$(basename $1).expected

metaflac --list "$1" > "${ACTUAL}"
cmp "${ACTUAL}" "${EXPECTED}"


# function check() {
# }

# check OUT/01-track.flac
# check OUT/02-track.flac