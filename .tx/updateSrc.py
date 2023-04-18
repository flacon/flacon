#!/usr/bin/env python3

import os
import sys
import shutil
import subprocess
import xml.dom.minidom as minidom

TS_FILE = "../translations/src.flacon.ts"
TMP_FILE = "../translations/src.flacon.ts.ts"

os.environ["PATH"] = os.environ["PATH"] + ":/opt/homebrew/Cellar/qt@5/5.15.2/bin"

class Error(Exception):
        pass

def find_lupdate():
    candidates = [
        "lupdate-qt5",
        #"lupdate-qt4",
        "lupdate",
    ]

    for c in candidates:
        path = shutil.which(c)
        if path:
            return path

    raise Error("The lupdate program not found.")

def lupdate(input, ts_file, silent = False):
    args = [
        find_lupdate(),
        "-no-obsolete",
        "-locations", "none",
        input,
        "-ts", ts_file
    ]

    if silent:
        args.append("-silent")

    subprocess.run(args)

def set_extracomment(ts_file):

    tmp_file=f"{ts_file}.tmp.ts"

    xml = minidom.parse(ts_file)
    ts = xml.getElementsByTagName("TS")[0]
    for context in ts.getElementsByTagName("context"):
        for message in context.getElementsByTagName("message"):

            comment = message.getElementsByTagName("comment")
            extracomment = message.getElementsByTagName("extracomment")

            if not extracomment and comment and comment[0].firstChild.nodeValue:

                extracomment = xml.createElement("extracomment")
                text = xml.createTextNode(comment[0].firstChild.nodeValue)
                extracomment.appendChild(text)
                message.appendChild(extracomment)

    with open(tmp_file, "w") as w:
        xml.writexml(w)

    lupdate(tmp_file, ts_file, silent = True)
    os.remove(tmp_file)

if __name__ == "__main__":
    try:
        lupdate("..", TS_FILE)
        set_extracomment(TS_FILE)

    except KeyboardInterrupt:
        sys.exit(0)

    except Error as err:
        sys.exit(err)
        sys.exit(1)
