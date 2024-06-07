#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

# ignore errexit with `&& true`
getopt --test > /dev/null && true
if [[ $? -ne 4 ]]; then
    echo '[ERROR] getopt invocation failed.'
    exit 1
fi

function printHelp() {
    echo "Usage: ./build_container.sh [<options>]"
    echo "Options:"
    echo "    -h | --help               Print this help message"
    echo "    -r | --rebuild            Clean the build directory and initialise CMake again (default: false)"
    echo "    -o | --output=<dir>       Set the directory to use as the build output (default: ./build)"
    echo "    -i | --image=<image:tag>  Specify which image to use when building (default: psxmc:latest)"
}

# option --output/-o requires 1 argument
LONGOPTS=help,rebuild,output:,image:
OPTIONS=hro:i:

# -temporarily store output to be able to check for errors
# -activate quoting/enhanced mode (e.g. by writing out “--options”)
# -pass arguments only via   -- "$@"   to separate them correctly
# -if getopt fails, it complains itself to stdout
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@") || exit 2
# read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

rebuild=0
outDir="./build"
image="psxmc:latest"
# now enjoy the options in order and nicely split until we see --
while true; do
    case "$1" in
        -h|--help)
            printHelp
            exit 1
            ;;
        -r|--rebuild)
            rebuild=1
            shift
            ;;
        -o|--output)
            outDir="$2"
            shift 2
            ;;
        -i|--image)
            image="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "[ERROR] Unknown option encountered: $1"
            exit 3
            ;;
    esac
done

COMMAND="python3 scripts/asset_bundler.py"
if [ $rebuild -eq 0 ]; then
  COMMAND="$COMMAND && cmake --build $outDir"
else
  rm -r "$outDir"
  echo "Removed build directory at: $outDir"
  COMMAND="$COMMAND && cmake --preset=default . && cmake --build $outDir"
fi

docker run -it \
  --rm \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -w "/tmp/PSX-Minecraft" \
  "$image" \
  /bin/bash -c "$COMMAND"
