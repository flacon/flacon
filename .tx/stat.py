#!/usr/bin/env python3

import os
from pathlib import Path
from xml.etree import ElementTree as ET

BARS_LEN=100
BARS_CHAR=":"

class Translation:
    def __init__(self, fileName):
        self.file = str(fileName)
        self.name = fileName.name
        self.lang = ""
        self.unfinished = 0
        self.finished   = 0
        self.total   = 0
        self.percent = 0

        self.load(fileName)

    def load(self, fileName):
        tree = ET.parse(fileName)
        root = tree.getroot()
        for m in root.findall(".//translation"):
            self.total += 1
            if m.attrib.get("type") == "unfinished":
                self.unfinished += 1
            else:
                self.finished += 1

        if self.total:
            self.percent = self.finished / self.total * 100


def graph(percent):
    n = int(percent / 100 * BARS_LEN)
    res = BARS_CHAR * n
    res += " " * (BARS_LEN - n)
    return res

def printRow(name, done, total):
    percent = 0
    if tot:
        percent = done / total * 100

    print("%-20s | %s | %3d%%   %4s of %s" % (name, graph(percent), percent, done, total))

def scanFiles(rootDir):
    res = []
    for f in Path(rootDir).rglob('*.ts'):
        if f.name.startswith("src."):
            continue
        res.append(Translation(f))
    return res


if __name__ == "__main__":
    translations = scanFiles("..")
    translations.sort(reverse=True, key=lambda item: item.percent)

    tot = 0
    sum = 0
    for t in translations:
        sum += t.finished
        tot += t.total
        printRow(t.name, t.finished, t.total)

    print("_________________________________________")
    printRow("Average", int(sum / len(translations)), int(tot / len(translations)) )

