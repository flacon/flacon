#!/bin/bash

set -e

SRC_DIR=${1:-..}
BIN_DIR=${2:-${PATH}}

EXTPROGRAM_H=${SRC_DIR}/extprogram.h
WHICH=$(which which)


RES=0
for PROG in $(sed -n 's|.*static ExtProgram \*\(.*\)().*|\1|p' "${EXTPROGRAM_H}"); do
  PATH=$BIN_DIR $WHICH $PROG || RES=1
done
exit $RES