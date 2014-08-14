#!/bin/sh

ANALYZER=$(which clang++)
DIR=../clang_check

mkdir -p ${DIR} && \
cd ${DIR} && \
scan-build --use-analyzer=${ANALYZER}  cmake -DCMAKE_BUILD_TYPE=Debug  .. && \
scan-build --use-analyzer=${ANALYZER}  make 