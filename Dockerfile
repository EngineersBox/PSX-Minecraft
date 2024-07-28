# Build and run:
#   docker build \
#    [--build-arg="REPO_TARGET=<username/reponame>"]
#    [--build-arg="REPO_COMMIT_ISH=<branch/commit/tag>"]
#    [--build-arg="GCC_MIPSEL_ELF_TAG=<Lameguy64/PSn00bSDK release tag>"]
#    [--build-arg="CACHEBUST=$(date +%s)"]
#    -t <tag> \
#    -f Dockerfile .

# TODO: Use a slimmer distro than bloaty ubuntu
FROM ubuntu:22.04

ARG REPO_TARGET=Lameguy64/PSn00bSDK
ARG REPO_COMMIT_ISH=master
ARG GCC_MIPSEL_ELF_TAG=v0.24
ARG PSN00BSDK_LIBC_ALLOCATOR="TLSF"

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
    && apt-get install -y build-essential \
        gcc \
        g++ \
        gdb \
        clang-15 \
        clangd-15 \
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
        ccls \
    && apt-get clean

# Create clang(d) symlinks to avoid versioned binaries
RUN ln -s /usr/bin/clangd-15 /usr/bin/clangd \
    && ln -s /usr/bin/clang-cpp-15 /usr/bin/clang-cpp \
    && ln -s /usr/bin/clang-15 /usr/bin/clang \
    && ln -s /usr/bin/clang++-15 /usr/bin/clang++

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

ADD scripts/requirements.txt /opt/requirements.txt
RUN python3 -m pip install -r /opt/requirements.txt

# Add cclangd script for LSPs
ADD cclangd /usr/local/bin/cclangd

ENV PSN00BSDK_LIBS="/opt/psn00bsdk/lib/libpsn00b"
ENV PATH="$PATH:/opt/psn00bsdk/mipsel-none-elf-gcc/bin:/opt/psn00bsdk/bin"

WORKDIR /opt
# Docker cache avoidance to detect new commits
ARG CACHEBUST=0
# Clone SDK repo
RUN git clone "https://github.com/$REPO_TARGET.git"
# No capitals for you!
RUN mv PSn00bSDK psn00bsdk

WORKDIR /opt/psn00bsdk
# Checkout target branch/commit/tag
RUN git checkout "$REPO_COMMIT_ISH"
# Initialise submodule dependencies
RUN git submodule update --init --recursive
# Pull and unpack GCC
RUN wget "https://github.com/Lameguy64/PSn00bSDK/releases/download/$GCC_MIPSEL_ELF_TAG/gcc-mipsel-none-elf-12.3.0-linux.zip" \
    && unzip gcc-mipsel-none-elf-12.3.0-linux.zip -d /opt/psn00bsdk/mipsel-none-elf-gcc \
    && rm gcc-mipsel-none-elf-12.3.0-linux.zip
# Build the SDK
RUN cmake -D PSN00BSDK_LIBC_ALLOCATOR=$PSN00BSDK_LIBC_ALLOCATOR --preset default --install-prefix /opt/psn00bsdk . \
    && cmake --build ./build  \
    && cmake --install ./build

WORKDIR /
