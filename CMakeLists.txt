cmake_minimum_required(VERSION 3.21)

project(
	PSX-Minecraft
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "PSX Minecraft"
	HOMEPAGE_URL "https://github.com/EngineersBox/PSX-Minecraft"
)
# 0 = Debug, 3 = Release
set(COMPILE_OPTIMISATION_LEVEL 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fshort-enums -Wno-unused-parameter -Wno-comments")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O${COMPILE_OPTIMISATION_LEVEL}")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fanalyzer")
set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
configure_file(assets.xml _assets.xml)

# Add a build step to pack assets into a single .LZP file. Note that, since we
# are specifying dependencies, CMake can detect when source assets are edited
# and rebuild the archive automatically.
file(GLOB_RECURSE _assets ${ASSETS_DIR}/*.tim ${ASSETS_DIR}/*.smd)
list(LENGTH _assets asset_count)
if (NOT (asset_count EQUAL 0))
	add_custom_command(
		COMMAND    ${LZPACK} -y _assets.xml
		OUTPUT     assets.lzp
		BYPRODUCTS textures.qlp gui.qlp
		DEPENDS    ${_assets}
		COMMENT    "Building LZP archive"
	)
endif()

message("Globbed ${asset_count} assets:")
foreach (_asset ${_assets})
	message("- ${_asset}")
endforeach()

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
	DEPENDS PSXMC system.cnf # PS1 system config dependency
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

target_compile_definitions(PSXMC PUBLIC -DPSXMC_VERSION_STRING="PSXMC Infdev")
target_compile_definitions(PSXMC PUBLIC -DFPT_BITS=32 -DFPT_WBITS=20)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG=1)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_FPS=1)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_POS=1)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_DIR=0)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_FACING=1)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_WORLD=1)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_MEM=0)

# Include assets in binary
if (NOT (asset_count EQUAL 0))
	psn00bsdk_target_incbin(
		PSXMC PRIVATE _lz_resources
		${PROJECT_BINARY_DIR}/assets.lzp
	)
endif()

install(FILES ${PROJECT_BINARY_DIR}/PSXMC.exe TYPE BIN)
