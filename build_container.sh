#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

PWD="$(pwd)"
PROJECT_DIR_NAME="PSX-Minecraft"

# Ensure we work from the project base dir to avoid
# weird mounting behaviour when running container
case "$(basename "$PWD")" in
  "$PROJECT_DIR_NAME") ;;
  *)
    echo "[ERROR] This script must be run from the $PROJECT_DIR_NAME directory, not $PWD"
    exit 1
    ;;
esac

# Ignore errexit with `&& true`
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

# Note that options with a ':' require an argument
LONGOPTS=help,rebuild,output:,image:
OPTIONS=hro:i:

# 1. Temporarily store output to be able to check for errors
# 2. Activate quoting/enhanced mode (e.g. by writing out “--options”)
# 3. Pass arguments only via   -- "$@"   to separate them correctly
# 4. If getopt fails, it complains itself to stdout
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@") || exit 2
# Read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

rebuild=0
outDir="./build"
image="psxmc:latest"
# Handle options in order and nicely split until we see --
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

# Construct the command to run in the container,
# only invoking a cmake build recreation when the
# flag is set
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
