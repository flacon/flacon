#!/bin/sh
cppcheck -q  --enable=performance,portability,warning,style --library=qt ..
