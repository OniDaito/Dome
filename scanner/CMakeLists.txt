
cmake_minimum_required (VERSION 2.8) 
project (scanner) 

################################
# Set source files

set(SOURCE_FILES 
	src/uvc_camera.cpp
  src/colorspaces.c
  src/bmp.cpp
  src/main.cpp
)

################################################################
# GLIB 

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB_PKG glib-2.0)

if (GLIB_PKG_FOUND)
    message(Found glib-2.0)
	include_directories(${GLIB_PKG_INCLUDE_DIRS})
endif(GLIB_PKG_FOUND)

################################
# DEBUG mode

SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
#SET(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x")

################################
# Setup the exectutable

#link_directories(${EXTRA_LIBS})
#link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable (scanner
	${SOURCE_FILES} 
) 

include_directories( "./include"
)

#target_link_libraries(scanner 
#)

