#!/bin/env python3

import os
import subprocess

def generate(fileName, duration):
    tmp_file=f"{fileName}.tmp.wav"

    (min, sec, frm) = duration.split(":")

    duration = ((int(min) * 60 + int(sec)) * 75 + int(frm)) * 176400 / 4 / 75

    args = [
     "sox",
     "-c", "2",
     "-b", "16",
     "-r", "44100",
     "-R",
     "-n", tmp_file,
     "synth",
     "%ds" % duration,
     "whitenoise"
    ]
    #print(args)
    subprocess.run(args, check=True)
    r = open(tmp_file, 'rb')
    data = r.read(4 * 1024)
    r.close()

    n = data.index(b'data')
    w = open(fileName, 'wb')
    w.write(data[:n + 8])
    w.close()

    os.remove(tmp_file)


generate("Rush - Signals - 01 - Subdivisions.wav",    "5:34:55")
generate("Rush - Signals - 02 - The Analog Kid.wav",  "4:48:02")
generate("Rush - Signals - 03 - Chemistry.wav",       "4:58:05")
generate("Rush - Signals - 04 - Digital Man.wav",     "6:22:13")
generate("Rush - Signals - 05 - The Weapon.wav",      "6:24:32")
generate("Rush - Signals - 06 - New World Man.wav",   "3:43:08")
generate("Rush - Signals - 07 - Losing It.wav",       "4:53:40")
generate("Rush - Signals - 08 - Countdown.wav",       "1:00:00")
