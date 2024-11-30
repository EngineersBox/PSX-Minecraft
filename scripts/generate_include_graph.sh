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
    echo "Usage: ./generate_include_graph.sh [<options>]"
    echo "Options:"
    echo "    -h | --help               Print this help message"
    echo "    -f | --format=<type>      Output format of the graph. See https://graphviz.org/docs/outputs/ for valid values (default: svg)"
    echo "    -o | --output=<filename>  Name of the generated graph file (default: graph.svg)"
}

# Note that options with a ':' require an argument
LONGOPTS=help,format:,output:
OPTIONS=hf:o:

# 1. Temporarily store output to be able to check for errors
# 2. Activate quoting/enhanced mode (e.g. by writing out “--options”)
# 3. Pass arguments only via   -- "$@"   to separate them correctly
# 4. If getopt fails, it complains itself to stdout
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@") || exit 2
# Read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

output="graph.svg"
format="svg"
# Handle options in order and nicely split until we see --
while true; do
    case "$1" in
        -h|--help)
            printHelp
            exit 1
            ;;
        -o|--output)
            output="$2"
            shift 2
            ;;
        -f|--format)
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

echo "[INFO] Generating graph"
rm graph.dot || true
perl scripts/cinclude2dot.pl --quotetypes quote --src src > graph.dot
echo "[INFO] Analysing strongly connected components"
sccmap -v graph.dot
rm $output || true
echo "[INFO] Formatting graph into $format format at $output"
dot -T $format graph.dot > $output
