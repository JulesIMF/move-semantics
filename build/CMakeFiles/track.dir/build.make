# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/julesimf/source/MIPT/Ded4/tracking

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/julesimf/source/MIPT/Ded4/tracking/build

# Include any dependencies generated for this target.
include CMakeFiles/track.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/track.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/track.dir/flags.make

CMakeFiles/track.dir/main.cpp.o: CMakeFiles/track.dir/flags.make
CMakeFiles/track.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/julesimf/source/MIPT/Ded4/tracking/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/track.dir/main.cpp.o"
	/bin/clang++-12  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/track.dir/main.cpp.o -c /home/julesimf/source/MIPT/Ded4/tracking/main.cpp

CMakeFiles/track.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/track.dir/main.cpp.i"
	/bin/clang++-12 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/julesimf/source/MIPT/Ded4/tracking/main.cpp > CMakeFiles/track.dir/main.cpp.i

CMakeFiles/track.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/track.dir/main.cpp.s"
	/bin/clang++-12 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/julesimf/source/MIPT/Ded4/tracking/main.cpp -o CMakeFiles/track.dir/main.cpp.s

# Object files for target track
track_OBJECTS = \
"CMakeFiles/track.dir/main.cpp.o"

# External object files for target track
track_EXTERNAL_OBJECTS =

track: CMakeFiles/track.dir/main.cpp.o
track: CMakeFiles/track.dir/build.make
track: Tracker/bin/libTracker.a
track: CMakeFiles/track.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/julesimf/source/MIPT/Ded4/tracking/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable track"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/track.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/track.dir/build: track

.PHONY : CMakeFiles/track.dir/build

CMakeFiles/track.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/track.dir/cmake_clean.cmake
.PHONY : CMakeFiles/track.dir/clean

CMakeFiles/track.dir/depend:
	cd /home/julesimf/source/MIPT/Ded4/tracking/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/julesimf/source/MIPT/Ded4/tracking /home/julesimf/source/MIPT/Ded4/tracking /home/julesimf/source/MIPT/Ded4/tracking/build /home/julesimf/source/MIPT/Ded4/tracking/build /home/julesimf/source/MIPT/Ded4/tracking/build/CMakeFiles/track.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/track.dir/depend

