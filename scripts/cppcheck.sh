#!/bin/sh
cppcheck -q --force --enable=performance,portability,warning,style --library=qt ..
