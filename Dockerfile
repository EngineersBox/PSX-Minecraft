# Build and run:
#   docker build \
#    [--build-arg="REPO_TARGET=<branch/commit/tag>"]
#    -t <tag> \
#    -f Dockerfile .

FROM ubuntu:22.04

ARG REPO_TARGET=master

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
    && apt-get install -y build-essential \
        gcc \
        g++ \
        gdb \
        clang \
        make \
        ninja-build \
        autoconf \
        automake \
        libtool \
        valgrind \
        locales-all \
        dos2unix \
        rsync \
        tar \
        python3 \
        python3-pip \
        python3-dev \
        git \
        unzip \
        wget \
        gpg \
        ca-certificates \
    && apt-get clean

# Ensure we don't have an existing CMake installation
RUN apt-get --yes purge --auto-remove cmake

# Install dependencies
RUN apt update \
    && apt install -y software-properties-common lsb-release \
    && apt clean all

# Get a copy of Kitware's GPG signing key
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

# Add Kitware's repo to sources list
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null

# Install Kitware GPG signing key
RUN apt update \
    && rm /usr/share/keyrings/kitware-archive-keyring.gpg \
    && apt install kitware-archive-keyring

# Install latest CMake
RUN apt update \
    && apt install --yes cmake \
    && apt-get clean

RUN python3 -m pip install coloredlogs

ENV PSN00BSDK_LIBS="/opt/psn00bsdk/lib/libpsn00b"
ENV PATH="$PATH:/opt/psn00bsdk/mipsel-none-elf-gcc/bin:/opt/psn00bsdk/bin"

WORKDIR /opt
# Clone SDK repo
RUN git clone https://github.com/Lameguy64/PSn00bSDK.git
# No capitals for you!
RUN mv PSn00bSDK psn00bsdk

WORKDIR /opt/psn00bsdk
# Checkout target branch/commit/tag
RUN git checkout "$REPO_TARGET"
# Initialise submodule dependencies
RUN git submodule update --init --recursive
# Pull and unpack GCC
RUN wget https://github.com/Lameguy64/PSn00bSDK/releases/download/v0.24/gcc-mipsel-none-elf-12.3.0-linux.zip
RUN unzip gcc-mipsel-none-elf-12.3.0-linux.zip -d /opt/psn00bsdk/mipsel-none-elf-gcc
RUN rm gcc-mipsel-none-elf-12.3.0-linux.zip
# Build the SDK
RUN cmake --preset default --install-prefix /opt/psn00bsdk .
# Double build for some weird permissions issues
# RUN cmake --build ./build
RUN cmake --build ./build
RUN cmake --install ./build

WORKDIR /