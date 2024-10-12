#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::usrsctp::usrsctp" for configuration "Release"
set_property(TARGET unofficial::usrsctp::usrsctp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::usrsctp::usrsctp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ws2_32;iphlpapi"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/usrsctp.lib"
  )

list(APPEND _cmake_import_check_targets unofficial::usrsctp::usrsctp )
list(APPEND _cmake_import_check_files_for_unofficial::usrsctp::usrsctp "${_IMPORT_PREFIX}/lib/usrsctp.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
