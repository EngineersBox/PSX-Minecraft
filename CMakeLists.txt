# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	PSX-Minecraft
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "PSX Minecraft"
	HOMEPAGE_URL "https://github.com/EngineersBox/PSX-Minecraft"
)

psn00bsdk_add_executable(PSX_Minecraft GPREL main.c)

psn00bsdk_add_cd_image(
	iso      # Target name
	PSX_Minecraft # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS PSX_Minecraft system.cnf
)
