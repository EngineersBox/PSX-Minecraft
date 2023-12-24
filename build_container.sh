#!/usr/bin/env bash

SDK_LOCATION=$1

docker run -it \
  --rm \
  -v "$SDK_LOCATION:/opt/psn00bsdk" \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -w "/tmp/PSX-Minecraft" \
  clion/ubuntu/cpp-env:1.0 \
  /bin/bash -c 'python3 asset_bundler.py && cmake --build ./build'