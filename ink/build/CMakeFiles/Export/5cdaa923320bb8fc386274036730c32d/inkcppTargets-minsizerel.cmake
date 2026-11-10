#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "inkcpp" for configuration "MinSizeRel"
set_property(TARGET inkcpp APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(inkcpp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/ink/inkcpp.lib"
  )

list(APPEND _cmake_import_check_targets inkcpp )
list(APPEND _cmake_import_check_files_for_inkcpp "${_IMPORT_PREFIX}/lib/ink/inkcpp.lib" )

# Import target "inkcpp_compiler" for configuration "MinSizeRel"
set_property(TARGET inkcpp_compiler APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(inkcpp_compiler PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/ink/inkcpp_compiler.lib"
  )

list(APPEND _cmake_import_check_targets inkcpp_compiler )
list(APPEND _cmake_import_check_files_for_inkcpp_compiler "${_IMPORT_PREFIX}/lib/ink/inkcpp_compiler.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
