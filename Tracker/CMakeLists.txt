#+++
# 
# Copyright(c) 2021 JulesIMF, MIPT
#
# Module name :
#
#		CMakeLists.txt
#
# Abstract:
#
#		My default CMake.Allows to compile the project
#		changing only FILES(and other flags if needed).	
#
#---

#
# Settings
#

cmake_minimum_required(VERSION 3.16)
project(tracker LANGUAGES CXX)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(WARNING_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY bin)

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

set(CMAKE_CXX_FLAGS "${OPT} ${ASAN} -g")

#
# Building
#

include_directories(includes 
                    internal)

add_library(Tracker STATIC
    Tracker.cpp
    Colors.cpp
    Console.cpp
)

target_link_libraries(Tracker)