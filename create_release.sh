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
    echo "Usage: ./create_release.sh [<options>]"
    echo "Options:"
    echo "    -h | --help               Print this help message"
    echo "    -t | --type=<build type>  Set the build type as either 'release' or 'debug' [Default: 'release']"
}

# Note that options with a ':' require an argument
LONGOPTS=help,type:
OPTIONS=ht:
#
# 1. Temporarily store output to be able to check for errors
# 2. Activate quoting/enhanced mode (e.g. by writing out “--options”)
# 3. Pass arguments only via   -- "$@"   to separate them correctly
# 4. If getopt fails, it complains itself to stdout
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@") || exit 2
# Read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

type="release"
# Handle options in order and nicely split until we see --
while true; do
    case "$1" in
        -h|--help)
            printHelp
            exit 1
            ;;
        -t|--type)
            case "$2" in
                release|debug)
                    type="$2"
                    shift 2
                    ;;
                *)
                    echo "[ERROR] Unknown build type encountered: $2"
                    printHelp
                    exit 1
                    ;;
            esac
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

./build_container.sh

pushd build
tar_name="infdev_${type}_$(date -u +%Y_%m_%dT%H_%M_%S).tar.gz"
tar -czf "$tar_name" PSXMC.bin PSXMC.cue PSXMC.exe PSXMC.map PSXMC.elf
mv "$tar_name" ..
popd build
