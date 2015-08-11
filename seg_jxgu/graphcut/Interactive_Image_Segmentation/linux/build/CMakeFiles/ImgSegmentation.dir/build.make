# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build

# Include any dependencies generated for this target.
include CMakeFiles/ImgSegmentation.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ImgSegmentation.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ImgSegmentation.dir/flags.make

CMakeFiles/ImgSegmentation.dir/main.cpp.o: CMakeFiles/ImgSegmentation.dir/flags.make
CMakeFiles/ImgSegmentation.dir/main.cpp.o: ../main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ImgSegmentation.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ImgSegmentation.dir/main.cpp.o -c /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/main.cpp

CMakeFiles/ImgSegmentation.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ImgSegmentation.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/main.cpp > CMakeFiles/ImgSegmentation.dir/main.cpp.i

CMakeFiles/ImgSegmentation.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ImgSegmentation.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/main.cpp -o CMakeFiles/ImgSegmentation.dir/main.cpp.s

CMakeFiles/ImgSegmentation.dir/main.cpp.o.requires:
.PHONY : CMakeFiles/ImgSegmentation.dir/main.cpp.o.requires

CMakeFiles/ImgSegmentation.dir/main.cpp.o.provides: CMakeFiles/ImgSegmentation.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/ImgSegmentation.dir/build.make CMakeFiles/ImgSegmentation.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/ImgSegmentation.dir/main.cpp.o.provides

CMakeFiles/ImgSegmentation.dir/main.cpp.o.provides.build: CMakeFiles/ImgSegmentation.dir/main.cpp.o

# Object files for target ImgSegmentation
ImgSegmentation_OBJECTS = \
"CMakeFiles/ImgSegmentation.dir/main.cpp.o"

# External object files for target ImgSegmentation
ImgSegmentation_EXTERNAL_OBJECTS =

ImgSegmentation: CMakeFiles/ImgSegmentation.dir/main.cpp.o
ImgSegmentation: CMakeFiles/ImgSegmentation.dir/build.make
ImgSegmentation: /usr/local/lib/libopencv_videostab.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_superres.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_stitching.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_shape.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_photo.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_objdetect.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_hal.a
ImgSegmentation: /usr/local/lib/libopencv_calib3d.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_features2d.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_ml.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_highgui.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_videoio.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_imgcodecs.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_flann.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_video.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_imgproc.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_core.so.3.0.0
ImgSegmentation: /usr/local/lib/libopencv_hal.a
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libGLU.so
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libGL.so
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libSM.so
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libICE.so
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libX11.so
ImgSegmentation: /usr/lib/x86_64-linux-gnu/libXext.so
ImgSegmentation: /usr/local/share/OpenCV/3rdparty/lib/libippicv.a
ImgSegmentation: CMakeFiles/ImgSegmentation.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ImgSegmentation"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ImgSegmentation.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ImgSegmentation.dir/build: ImgSegmentation
.PHONY : CMakeFiles/ImgSegmentation.dir/build

CMakeFiles/ImgSegmentation.dir/requires: CMakeFiles/ImgSegmentation.dir/main.cpp.o.requires
.PHONY : CMakeFiles/ImgSegmentation.dir/requires

CMakeFiles/ImgSegmentation.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ImgSegmentation.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ImgSegmentation.dir/clean

CMakeFiles/ImgSegmentation.dir/depend:
	cd /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build /home/jxgu/github/superb-view-video-segmentation/seg_jxgu/graphcut/Interactive_Image_Segmentation/linux/build/CMakeFiles/ImgSegmentation.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ImgSegmentation.dir/depend

