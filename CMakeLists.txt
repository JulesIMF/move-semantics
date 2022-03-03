#+++
# 
# Copyright (c) 2021 JulesIMF, MIPT
#
# Module name:
#
#		CMakeLists.txt
#
# Abstract:
#
#		My default CMake. Allows to compile the project
#		changing only FILES (and other flags if needed).
#		
#		Build this project with "make" or "make jimp".#		
#
#---

#
# Settings
#

cmake_minimum_required(VERSION 3.16)
project(jimp LANGUAGES CXX)
# set(CMAKE_CXX_COMPILER )
set(CMAKE_CXX_STANDARD 17)
set(WARNING_FLAGS "-Wno-multichar -Wno-unused-result -Wno-narrowing")
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

#
# Definitions
#

# add_definitions(NDEBUG)

set(OPT "-O2")
set(SANITIZE 0)

if(SANITIZE)
    set(ASAN "-fsanitize=address")
else()
    set(ASAN "")
endif()

set(CMAKE_CXX_FLAGS "${OPT} ${WARNING_FLAGS} ${ASAN} -g")
	
#
# Building
#

find_package(SFML 2 COMPONENTS system window graphics network audio REQUIRED)

add_subdirectory(Tracker)

include_directories(Tracker/includes)

add_executable(track
        main.cpp
) 

find_library(Tracker libTracker.a Tracker/bin)

target_link_libraries(track Tracker)