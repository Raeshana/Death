# Install script for directory: C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/inkcpp")
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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/shared/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_compiler/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_cl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/unreal/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp/Debug/inkcpp.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp/Release/inkcpp.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp/MinSizeRel/inkcpp.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp/RelWithDebInfo/inkcpp.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ink" TYPE FILE FILES
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/choice.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/functional.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/globals.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/list.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/runner.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/snapshot.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/story.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/story_ptr.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/traits.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp/include/types.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ink" TYPE FILE FILES
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/shared/public/config.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/shared/public/system.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/shared/public/version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_compiler/Debug/inkcpp_compiler.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_compiler/Release/inkcpp_compiler.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_compiler/MinSizeRel/inkcpp_compiler.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ink" TYPE STATIC_LIBRARY FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcpp_compiler/RelWithDebInfo/inkcpp_compiler.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ink" TYPE FILE FILES
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp_compiler/include/compilation_results.h"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/inkcpp_compiler/include/compiler.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp/inkcppTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp/inkcppTargets.cmake"
         "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp/inkcppTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp/inkcppTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/CMakeFiles/Export/5cdaa923320bb8fc386274036730c32d/inkcppTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/inkcpp" TYPE FILE FILES
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcppConfig.cmake"
    "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/inkcppConfigVersion.cmake"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/raesh/15466/15-466-f26-base4/ink/inkcpp/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
