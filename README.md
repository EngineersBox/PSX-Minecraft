# PSX-Minecraft

Remake of Minecraft for PS1 with PSn00bSDK

**Table of Contents**
<!-- TOC -->
* [PSX-Minecraft](#psx-minecraft)
  * [Dependencies](#dependencies)
  * [References](#references)
  * [Building](#building)
  * [Docker Environment](#docker-environment)
    * [Utility Build Script](#utility-build-script)
    * [CLion](#clion)
  * [Progress](#progress)
<!-- TOC -->

## Dependencies

* Emulator: [PCSX Redux](https://github.com/grumpycoders/pcsx-redux)
* PS1 Library: [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK)

## References

* [Inline programming reference](./docs/Inline%20Programming%20Reference.pdf)
* [LibPSn00b reference](./docs/LibPSn00b%20Reference.pdf)
* [Playstation specifications psx-spx](https://psx-spx.consoledev.net/kernelbios/)
  (Everything from hardware to software in great detail)
* [PsyQ Docs and examples](https://psx.arthus.net/sdk/Psy-Q/)

## Building

Configuring the project can be done with

```shell
cmake --preset default .
```

Then the CD image can be built with

```shell
cmake --build ./build
```

This will create several files in the `build` directory. Using the
PCSX Redux emulator, you can load the `build/PSXMC.bin` CD image
and run it.

## Docker Environment

Depending on the system you are building on (Windows/Linux/OSX) you may want to build locally,
however, in the case that you are on OSX and want the ease of a development environment in
Linux without the extra configuration headache, a Dockerfile has been provided that can be
used with the `build_container.sh` script to build the project.

First build the docker image, which will initialise a Linux distribution with all the necessary
tooling and build the SDK. The following are optional arguments you can provide:

* `--build-arg="REPO_TARGET=<user name>/<repo name>"` specify the namespaced repo (potential a fork) to build from. Defaults
  to `Lameguy64/PSn00bSDK`.
* `--build-arg="REPO_COMMIT_ISH=<branch/commit/tag>"` specify a build arg to target a specific branch/commit/tag of the SDK repo.
  Defaults to `master`.
* `--build-arg="GCC_MIPSEL_ELF_TAG=<Lameguy64/PSn00bSDK release tag>"` specify a release tag to pull the GCC build from.
  Defaults to `v0.24`.
* `--build-arg=PSN00BSDK_LIBC_ALLOCATOR=<CUSTOM | AAF | TLSF>` specify an allocator to use in the SDK build, must be one
  of `CUSTOM`, `AAF` (allocated-first-fit), or `TLSF` (two-level-segregated-fit). This currently requires `EngineersBox/PSn00bSDK`
  fork with `multiple-allocators` branch. Defaults to `TLSF`.
* `--build-arg="CACHEBUST=$(date +%s)"` force a rebuild of the repo. Defaults to `0`.

Using this, we can build as follows:

```shell
docker build -t psxmc:latest -f Dockerfile .
```

### Utility Build Script

Instead, you can use the utility script `build_container.sh` to bundle assets and compile the
project, it essentially does the same with some extra controls. Usage is defined as such:

```shell
Usage: ./build_container.sh [-h|--help] [-r|--rebuild] [-o|--output=<dir>] [-i|--image=<image:tag>]
Options:
    -h | --help:              Print this help message
    -r | --rebuild:           Clean the build directory and initialise CMake again (default: false)
    -o | --output=<dir>:      Set the directory to use as the build output (default: ./build)
    -i | --image=<image:tag>: Specify which image to use when building (default: psxmc:latest)
```

Initially, you'll want to run it as `./build_container.sh --output=./build --rebuild`, but then for
later rebuilds when making changes to code, just run `./build_container.sh`. Include `--output=<directory>`
if you want to use a different directory than the default. Note that you'll need to use this option every
time you run the command in that case.

### CLion

You can use the Dockerfile to double as a build environment for CLion so that you can run
CMake within the Docker context attached to CLion. To the following to set this up:

1. Open up settings and go to `Build, Execution, Deployment > Toolchains`
2. Create a new Docker toolchain and select the image you had build before
3. Open the container settings
4. Set the CMake option to `Docker CMake`
5. Go to `Build, Execution, Deployment > CMake`
6. Create a new profile
7. Set the toolchain to the Docker toolchain you just created
8. Add `--preset default` to the CMake options field
9. Exit settings
10. Verify that the Docker daemon is running on your local
11. Reload CMake project

## Progress

Current work is implementing frustum culling and ordered visibility culling with ordered mesh
generation during binary greedy meshing for offset based culling.

![PSXMC](./docs/PSXMC.png)
![PSXMC](./docs/PSXMC-1.png)
![PSXMC](./docs/PSXMC-2.png)
