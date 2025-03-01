# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/luo980/gits/cam_utils

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/luo980/gits/cam_utils/build

# Include any dependencies generated for this target.
include CMakeFiles/cam_utils.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/cam_utils.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/cam_utils.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cam_utils.dir/flags.make

CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o: CMakeFiles/cam_utils.dir/flags.make
CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o: /home/luo980/gits/cam_utils/src/cam_sdl3_httpd.cpp
CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o: CMakeFiles/cam_utils.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/luo980/gits/cam_utils/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o -MF CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o.d -o CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o -c /home/luo980/gits/cam_utils/src/cam_sdl3_httpd.cpp

CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/luo980/gits/cam_utils/src/cam_sdl3_httpd.cpp > CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.i

CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/luo980/gits/cam_utils/src/cam_sdl3_httpd.cpp -o CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.s

# Object files for target cam_utils
cam_utils_OBJECTS = \
"CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o"

# External object files for target cam_utils
cam_utils_EXTERNAL_OBJECTS =

cam_utils: CMakeFiles/cam_utils.dir/src/cam_sdl3_httpd.cpp.o
cam_utils: CMakeFiles/cam_utils.dir/build.make
cam_utils: third_party/sdl/libSDL3.so.0.2.5
cam_utils: third_party/yaml-cpp/libyaml-cpp.a
cam_utils: /home/luo980/gits/cam_utils/third_party/sdl/src/dynapi/SDL_dynapi.sym
cam_utils: CMakeFiles/cam_utils.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/luo980/gits/cam_utils/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cam_utils"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cam_utils.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cam_utils.dir/build: cam_utils
.PHONY : CMakeFiles/cam_utils.dir/build

CMakeFiles/cam_utils.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cam_utils.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cam_utils.dir/clean

CMakeFiles/cam_utils.dir/depend:
	cd /home/luo980/gits/cam_utils/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/luo980/gits/cam_utils /home/luo980/gits/cam_utils /home/luo980/gits/cam_utils/build /home/luo980/gits/cam_utils/build /home/luo980/gits/cam_utils/build/CMakeFiles/cam_utils.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cam_utils.dir/depend

