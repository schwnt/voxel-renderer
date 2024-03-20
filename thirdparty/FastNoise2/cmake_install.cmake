# Install script for directory: D:/desktop/newfolder/thirdparty/FastNoise2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/GAME")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/desktop/newfolder/thirdparty/FastNoise2/src/Release/FastNoise.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/FastSIMD" TYPE FILE FILES
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/FastSIMD.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/FastSIMD_Config.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/FastSIMD_Export.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/FunctionList.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/InlInclude.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastSIMD/SIMDTypeList.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/FastNoise" TYPE FILE FILES
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/FastNoise.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/FastNoise_C.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/FastNoise_Config.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/FastNoise_Export.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Metadata.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/SmartNode.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/FastNoise/Generators" TYPE FILE FILES
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/BasicGenerators.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Blends.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Cellular.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/DomainWarp.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/DomainWarpFractal.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Fractal.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Generator.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Modifiers.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Perlin.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Simplex.h"
    "D:/desktop/newfolder/thirdparty/FastNoise2/src/../include/FastNoise/Generators/Value.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2" TYPE FILE FILES
    "D:/desktop/newfolder/thirdparty/FastNoise2/generated/FastNoise2Config.cmake"
    "D:/desktop/newfolder/thirdparty/FastNoise2/generated/FastNoise2ConfigVersion.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2/FastNoise2Targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2/FastNoise2Targets.cmake"
         "D:/desktop/newfolder/thirdparty/FastNoise2/CMakeFiles/Export/lib/cmake/FastNoise2/FastNoise2Targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2/FastNoise2Targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2/FastNoise2Targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2" TYPE FILE FILES "D:/desktop/newfolder/thirdparty/FastNoise2/CMakeFiles/Export/lib/cmake/FastNoise2/FastNoise2Targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/FastNoise2" TYPE FILE FILES "D:/desktop/newfolder/thirdparty/FastNoise2/CMakeFiles/Export/lib/cmake/FastNoise2/FastNoise2Targets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE DIRECTORY FILES "D:/desktop/newfolder/thirdparty/FastNoise2/pdb-files/Release/")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/desktop/newfolder/thirdparty/FastNoise2/src/cmake_install.cmake")

endif()

