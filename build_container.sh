#!/usr/bin/env bash

SDK_LOCATION=$1
IMAGE=${2:-"clion/ubuntu/cpp-env:1.0"}

docker run -it \
  --rm \
  -v "$SDK_LOCATION:/opt/psn00bsdk" \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -w "/tmp/PSX-Minecraft" \
  "$IMAGE" \
  /bin/bash -c 'python3 asset_bundler.py && cmake --build ./build'