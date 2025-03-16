#!/usr/bin/env python3

import subprocess
import os.path
import sys

BIN = "./flacon"
CUE_FILE="disk.cue"

if not os.path.isfile(BIN):
    print(f"Error: Executable file '{BIN}' not found", file=sys.stderr)
    sys.exit(1)


if not os.path.isfile(CUE_FILE):
    print(f"Error: CUE file '{CUE_FILE}' not found", file=sys.stderr)
    sys.exit(2)


def get_seed(str):
    b = str.index("[")
    b = str.index("=", b)
    e = str.index(",", b)
    return str[b+1:e]

def get_ratio(str):
    b = str.index("[")
    b = str.index(",", b)
    b = str.index("=", b)
    e = str.index("]", b)
    return str[b+1:e]


args = [
    "zzuf",
    "-s", "20",
    "-c",
    "-v",

    BIN,
    "-s",
    "disk.cue"
    ]
print(f"run {" ".join(args)}")
result = subprocess.run(args, stderr=subprocess.PIPE)
out = result.stderr.decode("UTF-8")
out = out.split("\n")
out = [w for w in out if w.startswith("zzuf")]
out = out[-1]
#print("===============")
#print(out)
#print("---------------")



seed  = get_seed(out)
ratio = get_ratio(out)

corrupted_file = f"corrupted-s{seed}-r{ratio}.cue"
r = open("disk.cue", "r")
w = open(corrupted_file, "w")
args = [
    "zzuf",
    "-s", seed,
    "-r", ratio
]
subprocess.run(args, stdin=r, stdout=w)
print(f"wrote corrupted file '{corrupted_file}'")
