#!/usr/bin/env python3

VERSIOIN = "0.1"

import sys
import argparse
import re

def showVersion():
	print("indexcalc.py   %s" % VERSIOIN)


class CueIndex:
    def __init__(self, str = "00:00:00", cdQuality = True):
        self._cdValue = 0
        self._hiValue = 0

        sl = re.split("\D", str)

        min = int(sl[0])
        sec = int(sl[1])
        frm = int(sl[2].ljust(2, '0'))
        msec = int(sl[2].ljust(3, '0'))

        self._cdValue = (min * 60 + sec) * 75 + frm
        self._hiValue = (min * 60 + sec) * 1000 + msec

    def fromMillisec(self, msec):
        self._hiValue = msec
        self._cdValue = msec / 1000 * 75

    def __str__(self):
        res = ""
        hMin  = int(self._hiValue / (60 * 1000))
        hSec  = int((self._hiValue - hMin * 60 * 1000) / 1000)
        hMsec = int(self._hiValue - (hMin * 60 + hSec) * 1000)

        res += "HD: %02d:%02d.%03d  " %(hMin, hSec, hMsec)

        cMin = int(self._cdValue / (60 * 75))
        cSec = int((self._cdValue - cMin * 60 * 75) / 75)
        cFrm = int(self._cdValue - (cMin * 60 + cSec) * 75)

        res += "CD: %02d:%02d:%02d" %(cMin, cSec, cFrm)

        return res


    def __add__(self, other):
        res = CueIndex()
        res._cdValue = self._cdValue + other._cdValue
        res._hiValue = self._hiValue + other._hiValue
        return res


    def __sub__(self, other):
        res = CueIndex()
        res._cdValue = self._cdValue - other._cdValue
        res._hiValue = self._hiValue - other._hiValue
        return res



#######################################
#
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Calculator for CUE indexes.")

    parser.add_argument('-V', "--version",  action='store_true', help="output version information and exit")

    parser.add_argument('index1', metavar='INDEX_1', help='CUE index', type=CueIndex)
    parser.add_argument('operator', metavar='OPERATOR', help='operator', choices=['+', '-'])
    parser.add_argument('index2', metavar='INDEX_2', help='CUE index', type=CueIndex)
    #parser.add_argument('expression', metavar='EPR', nargs='+', help='an expression for calculation')

    args = parser.parse_args()

    if args.version:
        showVersion()
        sys.exit(0)

    try:
        if args.operator == "+":
            print( args.index1 + args.index2)
        else:
            print( args.index1 - args.index2)

    except KeyboardInterrupt:
        exit(0)
