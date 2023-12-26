# Build and run:
#   docker build -t clion/ubuntu/cpp-env:1.0 -f Dockerfile.cpp-env-ubuntu .

FROM ubuntu:22.04

ARG SDK_LOCATION

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
  && apt-get install -y build-essential

# Ensure we don't have an existing CMake installation
RUN apt-get --yes purge --auto-remove cmake

# Install dependencies
RUN apt update \
    && apt install -y software-properties-common lsb-release \
    && apt clean all

# Get a copy of Kitware's GPG signing key
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 42D5A192B819C5DA
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

# Add Kitware's repo to sources list
RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"

# Install Kitware GPG signing key
RUN apt update \
    && apt install kitware-archive-keyring \
    && rm /etc/apt/trusted.gpg.d/kitware.gpg

# Install latest CMake
RUN apt update \
    && apt install --yes cmake \
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
  && apt-get clean

RUN python3 -m pip install coloredlogs

ENV PSN00BSDK_LIBS="/opt/psn00bsdk/lib/libpsn00b"
ENV PATH="$PATH:/opt/psn00bsdk/bin"
