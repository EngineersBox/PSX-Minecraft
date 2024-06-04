#!/usr/bin/env bash

IMAGE=${1:-"psxmc:latest"}

docker run -it \
  --rm \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -w "/tmp/PSX-Minecraft" \
  "$IMAGE" \
  /bin/bash -c 'python3 scripts/asset_bundler.py && cmake --build ./build'
