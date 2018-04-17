#!/bin/sh


TEMPLATE="{file}\t{line}\t{severity}\t{id} {message}"
cd ..
cppcheck -q --force --enable=performance,portability,warning,style --library=qt --template="${TEMPLATE}" . 2>&1 | tee cppcheck.pro.user.tasks

