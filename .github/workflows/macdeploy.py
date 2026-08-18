#!/usr/bin/env python3

SCRIPT_VERSION = "0.2.0"


import argparse
import glob
import os
import sys
import subprocess
import fnmatch

VERBOSE  = False
PROG = os.path.basename(sys.argv[0])

SYS_LIBS = [
    "/System/*",
    "/usr/lib/libobjc.*.dylib",
    "/usr/lib/libSystem.*.dylib",
    "/usr/lib/libiconv.2.dylib",
    "/usr/lib/libncurses.5.4.dylib",
    "/usr/lib/libc++.1.dylib",
    "/usr/lib/libz.1.dylib",
    "/usr/lib/libbz*.dylib",
    "/usr/lib/libxar.*.dylib",
    "/usr/lib/libcups.*.dylib",
    "/usr/lib/libresolv.*.dylib",
]


class Error(Exception):
        pass

#############################
#
#############################
def check_libs(args):
    bundleDir = args.bundle_dir

    if VERBOSE:
        print(f"Check libraries in {bundleDir}")

    if not os.path.isdir(bundleDir):
        raise Error(f"can't open input directory: {bundleDir} (No such directory)")

    errors = []

    def checkLib(fileName, lib):
        f = lib.replace("@executable_path",  f"{bundleDir}/Contents/MacOS")

        if not os.path.exists(f):
            errors.append(f"Library not found: {f}")

    def isSystemLib(lib):
        for s in SYS_LIBS:
            if fnmatch.fnmatchcase(lib, s):
                return True
        return False

    res = True
    for (dirpath, dirnames, filenames) in os.walk(bundleDir):

        for f in filenames:
            errors = []
            fileName = f"{dirpath}/{f}"
            lines = subprocess.check_output(["otool", "-L", fileName])
            lines = lines.split(b"\n")

            if VERBOSE:
                print(f" • {fileName}")

            for line in lines[1:]:
                line = line.decode("UTF-8").strip()

                if line == "":
                    continue

                lib = line[:line.index("(")].strip()

                if isSystemLib(lib):
                    continue


                suffix = fileName.removeprefix(f"{bundleDir}/Contents/PlugIns/")
                if lib.endswith(suffix):
                    continue

                if os.path.basename(lib) == os.path.basename(fileName):
                    continue

                if lib.startswith("@executable_path/../Frameworks/"):
                    checkLib(fileName, lib)
                    continue

                errors.append(f"Non local libreary - {lib}")

            if errors:
                res = False
                print(f"{fileName}")
                for e in errors:
                    print(f"- {e}")

    if not res:
        sys.exit(2)


if __name__ == "__main__":

    argv = []
    for a in sys.argv[1:]:
        argv += [a]
        if not a.startswith("-"):
            break

    cmdArgv = sys.argv[len(argv) + 1:]


    parser = argparse.ArgumentParser(
        description = "A tool to help deploy an application on a mac.")

    parser.add_argument('-V', '--version',
        action='version',
        version='%(prog)s ' + SCRIPT_VERSION)

    parser.add_argument('-v', "--verbose",
        action='store_true',
        help="enable verbose mode")

    subparsers = parser.add_subparsers(help='the program supports the following commands:', required = True)

    # Check libs .................
    parser_check_libs = subparsers.add_parser('check-libs', help='check binaries and .dylib files')
    parser_check_libs.set_defaults(func=check_libs)
    parser_check_libs.add_argument("bundle_dir", help=".app bundle directory")


    args = parser.parse_args()
    VERBOSE = args.verbose



    try:
        args.func(args)

    except KeyboardInterrupt:
        sys.exit(0)

    except Error as e:
        print(e, file=sys.stderr)
        sys.exit(1)



