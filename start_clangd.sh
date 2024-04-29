#!/bin/sh

CONTAINER_NAME="$1"
PROJECT_ROOT_PATH="$2"

docker run \
    -i \
    --rm \
    -v"$(pwd):/tmp/PSX-Minecraft" \
    -w "/tmp/PSX-Minecraft" \
    -e PSN00BSDK_LIBS=/opt/psn00bsdk/lib/libpsn00b \
    --name "$CONTAINER_NAME" \
    psxmc:latest \
    /usr/bin/clangd \
    --compile-commands-dir="build" \
    --path-mappings="$PROJECT_ROOT_PATH=/tmp/PSX-Minecraft" \
    --background-index \
    --log=verbose \
    2>/dev/null
