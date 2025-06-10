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
    echo "Usage: ./build_image.sh [<options>]"
    echo "Options:"
    echo "    -h | --help                 Print this help message"
    echo "    -c | --commit=<commit ish>  SDK commit-ish to check out (default: multiple-allocators)"
    echo "    -r | --repo=<SDK repo>      Namespaced SDK on GitHub (default: EngineersBox/PSn00bSDK)"
    echo "    -a | --allocator=<type>     Allocator type to use if using the EngineersBox/PSn00bSDK repo with multiple-allocators (default: TLSF)"
    echo "    -i | --image=<image:tag>    Specify which image to use when building (default: psxmc:latest)"
}

# Note that options with a ':' require an argument
LONGOPTS=help,commit:,repo:,allocator:,image:
OPTIONS=hc:r:a:i:

# 1. Temporarily store output to be able to check for errors
# 2. Activate quoting/enhanced mode (e.g. by writing out “--options”)
# 3. Pass arguments only via   -- "$@"   to separate them correctly
# 4. If getopt fails, it complains itself to stdout
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@") || exit 2
# Read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

commit="multiple-allocators"
repo="EngineersBox/PSn00bSDK"
allocator="TLSF"
image="psxmc:latest"
# Handle options in order and nicely split until we see --
while true; do
    case "$1" in
        -h|--help)
            printHelp
            exit 1
            ;;
        -c|--commit)
            commit="$2"
            shift 2
            ;;
        -r|--repo)
            repo="$2"
            shift 2
            ;;
        -a|--allocator)
            allocator="$2"
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

docker build "$image" \
    --build-arg="REPO_TARGET=$repo" \
    --build-arg="REPO_COMMIT_ISH=$commit" \
    --build-arg="PSN00BSDK_LIBC_ALLOCATOR=$allocator" \
    -f Dockerfile .
