#!/bin/bash

set -e

SRC_DIR=${1:-..}
BIN_DIR=${2:-${PATH}}

EXTPROGRAM_H=${SRC_DIR}/extprogram.h
WHICH=$(which which)

PROGS=$(sed -n 's|.*static ExtProgram \*\(.*\)().*|\1|p' "${EXTPROGRAM_H}")
RES=0
for PROG in $PROGS; do
  if PATH=$BIN_DIR $WHICH -s $PROG; then
    printf "%-20s  %s\n" $PROG "OK"
  else
    printf "%-20s  %s\n" $PROG "NOT FOUND"
    RES=1
  fi
done
exit $RES
