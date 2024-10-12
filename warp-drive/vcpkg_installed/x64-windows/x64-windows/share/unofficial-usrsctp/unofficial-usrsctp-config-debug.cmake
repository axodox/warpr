#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::usrsctp::usrsctp" for configuration "Debug"
set_property(TARGET unofficial::usrsctp::usrsctp APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::usrsctp::usrsctp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "ws2_32;iphlpapi"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/usrsctp.lib"
  )

list(APPEND _cmake_import_check_targets unofficial::usrsctp::usrsctp )
list(APPEND _cmake_import_check_files_for_unofficial::usrsctp::usrsctp "${_IMPORT_PREFIX}/debug/lib/usrsctp.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
