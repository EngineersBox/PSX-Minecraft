#!/bin/sh

CONTAINER_NAME="$1"
PROJECT_ROOT_PATH="$2"
# "/tmp/PSX-Minecraft/>$PROJECT_ROOT_PATH/"
# read -d '' INIT_JSON << EOF || true
# {
#     "clang": {
#         "pathMappings": [
#             "/tmp/PSX-Minecraft/>$PROJECT_ROOT_PATH/"
#         ]
#     },
#     "compilationDatabaseDirectory": "$PROJECT_ROOT_PATH/build",
#     "index": {
#         "onChange": true
#     }
# }
# EOF

docker run \
    -i \
    --rm \
    -v"$PROJECT_ROOT_PATH:$PROJECT_ROOT_PATH" \
    -w "$PROJECT_ROOT_PATH" \
    -e PSN00BSDK_LIBS=/opt/psn00bsdk/lib/libpsn00b \
    --name "$CONTAINER_NAME" \
    psxmc:latest \
    /bin/bash -c "ln -s $PROJECT_ROOT_PATH /tmp/PSX-Minecraft && ln -s $PROJECT_ROOT_PATH/.clangd /opt/psn00bsdk/include/libpsn00b/.clangd && /usr/bin/clangd \
    --compile-commands-dir=\"${PROJECT_ROOT_PATH}/build\" \
    --background-index"
    # --log=verbose"

    # /usr/bin/ccls \
    # --init="$INIT_JSON" \
    # -v=2
