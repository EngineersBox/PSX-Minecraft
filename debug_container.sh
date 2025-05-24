#!/usr/bin/env bash

IMAGE=${1:-"psxmc:latest"}

docker run -it \
  --rm \
  --add-host=host.docker.internal:host-gateway \
  -v "$(pwd):/tmp/PSX-Minecraft" \
  -v "$(pwd):$(pwd)" \
  -w "$(pwd)" \
  --name "psx_minecraft" \
  "$IMAGE" \
  /bin/bash gdb -ix .gdbinit
