#!/bin/bash

set -e

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )
METAINFO_FILE="${DIR}/misc/com.github.Flacon.metainfo.xml.in"
CMAKE_FILE="${DIR}/CMakeLists.txt"

MAJOR_VERSION=$(grep "set(MAJOR_VERSION"  ${CMAKE_FILE} | sed -e "s|.*_VERSION *\(.*\) *)|\1|")
MINOR_VERSION=$(grep "set(MINOR_VERSION"  ${CMAKE_FILE} | sed -e "s|.*_VERSION *\(.*\) *)|\1|")
PATCH_VERSION=$(grep "set(PATCH_VERSION"  ${CMAKE_FILE} | sed -e "s|.*_VERSION *\(.*\) *)|\1|")

if [[ "${MINOR_VERSION}" = "99" ]]; then
  echo "It's a beta release so don't check"
  exit 0
fi

if grep -q -e "<release date=\"[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\" version=\"${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}\">" ${METAINFO_FILE}; then
  exit 0
fi

echo "Incorrect misc/com.github.Flacon.metainfo.xml.in file" >&2
grep -e "<release " ${METAINFO_FILE} >&2

exit 1
