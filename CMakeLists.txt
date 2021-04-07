# - Main CMake buildscript for building Samba on Linux

# - Set CMake requirements then declare project
cmake_minimum_required(VERSION 3.2)
project(LinuxSamba VERSION 1.0)

#   Let's see what's happening
set(CMAKE_VERBOSE_MAKEFILE ON)

# - Don't allow C++ Compiler Vendor Extensions
set(CMAKE_CXX_EXTENSIONS OFF)

# - Set C++ Standard directly to c++17
set(CMAKE_CXX_STANDARD 17)

# - make sure we have a clean build
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wfatal-errors -Wshadow -pedantic")

# - check some OS dependent stuff
INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES("sys/sysctl.h" HAVE_SYS_SYSCTL_H)

if (WXWIDGETS)
  add_compile_definitions(WXWIDGETS UNIX)

  # - Find wxWidgets
  set(wxWidgets_CONFIGURATION mswu)
  find_package(wxWidgets COMPONENTS core base REQUIRED)
  include(${wxWidgets_USE_FILE})
else()
  # - Find OpenGL
  add_compile_definitions(ForOpiumGL UNIX)
  find_package(GLUT  REQUIRED)
  add_library(glfw3 STATIC IMPORTED)
  set_target_properties(glfw3 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libglfw3.a"
  )
  find_library(COREVIDEO_LIBRARY CoreVideo)
endif()

# - Add the main library and other subdirectories to the build
add_subdirectory(lib)
add_subdirectory(samba)
add_subdirectory(tango)
