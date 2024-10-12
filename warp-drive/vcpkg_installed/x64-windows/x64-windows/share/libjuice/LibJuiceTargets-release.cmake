#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "LibJuice::LibJuice" for configuration "Release"
set_property(TARGET LibJuice::LibJuice APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LibJuice::LibJuice PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/juice.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/juice.dll"
  )

list(APPEND _cmake_import_check_targets LibJuice::LibJuice )
list(APPEND _cmake_import_check_files_for_LibJuice::LibJuice "${_IMPORT_PREFIX}/lib/juice.lib" "${_IMPORT_PREFIX}/bin/juice.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
