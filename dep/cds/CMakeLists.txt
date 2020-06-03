# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Gather source files.
GetFilesWithSourceGroups(GLOB_RECURSE CDS_FILES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp cds/*.h)

# Define library target.
# add_library(cds ${CDS_FILES})

# Set include dirs for this library (done last, so it's not inherited by subprojects like Tervel, NBDS).
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# set_target_properties(cds PROPERTIES LINKER_LANGUAGE CXX)

add_library(cds STATIC ${CDS_FILES})

target_include_directories(cds
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
	
set_target_properties(cds
    PROPERTIES
      FOLDER
        "dep")
