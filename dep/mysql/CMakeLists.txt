
if (NOT MYSQL_FOUND)
  message(FATAL_ERROR "MySQL wasn't found on your system but it's required to build the servers!")
endif()

add_library(mysql STATIC IMPORTED GLOBAL)

set_target_properties(mysql
  PROPERTIES
    IMPORTED_LOCATION
      "${MYSQL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES
      "${MYSQL_INCLUDE_DIR}")

#install(TARGETS mysql RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")