#!/usr/bin/env bash

IMAGE=${1:-"psxmc:latest"}

docker run -id \
  --rm \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -v "$(pwd):$(pwd)" \
  -w "$(pwd)" \
  --name "psx_minecraft" \
  "$IMAGE" \
  /bin/bash