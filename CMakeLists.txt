cmake_minimum_required(VERSION 3.21)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)

project(
	PSX-Minecraft
	LANGUAGES    C CXX ASM
	VERSION      0.1.0
	DESCRIPTION  "PSX Minecraft"
	HOMEPAGE_URL "https://github.com/EngineersBox/PSX-Minecraft"
)
# 0 = Debug, 3 = Release
set(COMPILE_OPTIMISATION_LEVEL 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fshort-enums -Wunused-parameter -Wno-expansion-to-defined")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O${COMPILE_OPTIMISATION_LEVEL} -g")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fanalyzer")
set(CMAKE_C_INCLUDE_WHAT_YOU_USE "include-what-you-use;-w;-Xiwyu;--verbose=7")
include(${PROJECT_SOURCE_DIR}/bundles.cmake)
ConstructAssetBundles()

# Include source content
file(GLOB_RECURSE PSXMC_SOURCES CONFIGURE_DEPENDS "src/*.[cs]")
file(GLOB_RECURSE PSXMC_HEADERS CONFIGURE_DEPENDS "src/*.h")

set(PSXMC_INCLUDE_DIRS "")
foreach (_headerFile ${PSXMC_HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list(APPEND PSXMC_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES PSXMC_INCLUDE_DIRS)
message("Include dirs:")
foreach (_include_dir ${PSXMC_INCLUDE_DIRS})
	message("- ${_include_dir}")
endforeach()

# Mark executable
psn00bsdk_add_executable(PSXMC GPREL ${PSXMC_SOURCES})
target_include_directories(PSXMC PRIVATE ${PSXMC_INCLUDE_DIRS})

psn00bsdk_add_cd_image(
	iso # Target name
	PSXMC # Output file name (= <NAME>.bin + <NAME>.cue)
	iso.xml  # Path to CD layout file
    DEPENDS PSXMC system.cnf ${ASSET_BUNDLE_ARCHIVES}
)

# Include dependencies
include(FetchContent)

FetchContent_Declare(
		metalang99
		URL https://github.com/Hirrolot/metalang99/archive/refs/tags/v1.13.3.tar.gz
)
FetchContent_Declare(
		interface99
		URL https://github.com/Hirrolot/interface99/archive/refs/tags/v1.0.1.tar.gz
)

FetchContent_MakeAvailable(metalang99)
FetchContent_MakeAvailable(interface99)

target_link_libraries(PSXMC PRIVATE metalang99)
target_link_libraries(PSXMC PRIVATE interface99)

# Disable full macro expansion backtraces for Metalang99.
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(PSXMC PRIVATE -fmacro-backtrace-limit=1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(PSXMC PRIVATE -ftrack-macro-expansion=0)
endif()
target_compile_definitions(PSXMC PUBLIC -DFPT_BITS=32 -DFPT_WBITS=20)

include(${PROJECT_SOURCE_DIR}/compile_flags.cmake)

set_property(TARGET PSXMC PROPERTY C_STANDARD 23)

install(FILES ${PROJECT_BINARY_DIR}/PSXMC.exe TYPE BIN)
