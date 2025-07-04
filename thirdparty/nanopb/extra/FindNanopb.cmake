# This is an example script for use with CMake projects for locating and configuring
# the nanopb library.
#
# The following variables can be set and are optional:
#
#
#   PROTOBUF_SRC_ROOT_FOLDER - When compiling with MSVC, if this cache variable is set
#                              the protobuf-default VS project build locations
#                              (vsprojects/Debug & vsprojects/Release) will be searched
#                              for libraries and binaries.
#
#   NANOPB_IMPORT_DIRS       - List of additional directories to be searched for
#                              imported .proto files.
#   NANOPB_OPTIONS           - List of options passed to nanopb.
#
#   NANOPB_DEPENDS           - List of files to be used as dependencies
#                              for the generated source and header files. These
#                              files are not directly passed as options to
#                              nanopb but rather their directories.
#
#
#   NANOPB_GENERATE_CPP_APPEND_PATH - By default -I will be passed to protoc
#                                     for each directory where a proto file is referenced.
#                                     Set to FALSE if you want to disable this behaviour.
#
# Defines the following variables:
#
#   NANOPB_FOUND - Found the nanopb library (source&header files, generator tool, protoc compiler tool)
#   NANOPB_INCLUDE_DIRS - Include directories for Google Protocol Buffers
#
# The following cache variables are also available to set or use:
#   PROTOBUF_PROTOC_EXECUTABLE - The protoc compiler
#   NANOPB_GENERATOR_SOURCE_DIR - The nanopb generator source
#
#  ====================================================================
#
# NANOPB_GENERATE_CPP (public function)
# NANOPB_GENERATE_CPP(SRCS HDRS [RELPATH <root-path-of-proto-files>]
#                     <proto-files>...)
#   SRCS = Variable to define with autogenerated source files
#   HDRS = Variable to define with autogenerated header files
#   If you want to use relative paths in your import statements use the RELPATH
#   option. The argument to RELPATH should be the directory that all the
#   imports will be relative to.
#   When RELPATH is not specified then all proto files can be imported without
#   a path. 
#
#
#  ====================================================================
#  Example:
#
#   set(NANOPB_SRC_ROOT_FOLDER "/path/to/nanopb")
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${NANOPB_SRC_ROOT_FOLDER}/extra)
#   find_package( Nanopb REQUIRED )
#   include_directories(${NANOPB_INCLUDE_DIRS})
#
#   NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS foo.proto)
#
#   include_directories(${CMAKE_CURRENT_BINARY_DIR})
#   add_executable(bar bar.cc ${PROTO_SRCS} ${PROTO_HDRS})
#
#  Example with RELPATH:
#   Assume we have a layout like:
#    .../CMakeLists.txt
#    .../bar.cc
#    .../proto/
#    .../proto/foo.proto  (Which contains: import "sub/bar.proto"; )
#    .../proto/sub/bar.proto
#   Everything would be the same as the previous example, but the call to
#   NANOPB_GENERATE_CPP would change to:
# 
#   NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS RELPATH proto
#                       proto/foo.proto proto/sub/bar.proto)
# 
#  ====================================================================

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009-2011 Philip Lowman <philip@yhbt.com>
# Copyright 2008 Esben Mose Hansen, Ange Optimization ApS
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#=============================================================================
#
# Changes
# 2013.01.31 - Pavlo Ilin - used Modules/FindProtobuf.cmake from cmake 2.8.10 to
#                           write FindNanopb.cmake
#
#=============================================================================


function(NANOPB_GENERATE_CPP SRCS HDRS)
  cmake_parse_arguments(NANOPB_GENERATE_CPP "" "RELPATH" "" ${ARGN})
  if(NOT NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS)
    return()
  endif()

  if(NANOPB_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(APPEND _nanopb_include_path "-I${ABS_PATH}")
    endforeach()
  else()
    set(_nanopb_include_path "-I${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if(NANOPB_GENERATE_CPP_RELPATH)
    list(APPEND _nanopb_include_path "-I${NANOPB_GENERATE_CPP_RELPATH}")
  endif()

  if(DEFINED NANOPB_IMPORT_DIRS)
    foreach(DIR ${NANOPB_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(APPEND _nanopb_include_path -I ${ABS_PATH})
    endforeach()
  endif()

  list(REMOVE_DUPLICATES _nanopb_include_path)

  set(GENERATOR_PATH ${CMAKE_BINARY_DIR}/nanopb/generator)

  set(NANOPB_GENERATOR_EXECUTABLE ${GENERATOR_PATH}/nanopb_generator.py)
  set(NANOPB_GENERATOR_PLUGIN ${GENERATOR_PATH}/protoc-gen-nanopb)

  set(GENERATOR_CORE_DIR ${GENERATOR_PATH}/proto)
  set(GENERATOR_CORE_SRC
      ${GENERATOR_CORE_DIR}/nanopb.proto
      ${GENERATOR_CORE_DIR}/plugin.proto)

  # Treat the source diretory as immutable.
  #
  # Copy the generator directory to the build directory before
  # compiling python and proto files.  Fixes issues when using the
  # same build directory with different python/protobuf versions
  # as the binary build directory is discarded across builds.
  #
  add_custom_command(
      OUTPUT ${NANOPB_GENERATOR_EXECUTABLE} ${GENERATOR_CORE_SRC}
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ARGS ${NANOPB_GENERATOR_SOURCE_DIR} ${GENERATOR_PATH}
      VERBATIM)

  set(GENERATOR_CORE_PYTHON_SRC)
  foreach(FIL ${GENERATOR_CORE_SRC})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(FIL_WE ${FIL} NAME_WE)

      set(output "${GENERATOR_CORE_DIR}/${FIL_WE}_pb2.py")
      set(GENERATOR_CORE_PYTHON_SRC ${GENERATOR_CORE_PYTHON_SRC} ${output})
      add_custom_command(
        OUTPUT ${output}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS -I${GENERATOR_PATH}/proto
          --python_out=${GENERATOR_CORE_DIR} ${ABS_FIL}
        DEPENDS ${ABS_FIL}
        VERBATIM)
  endforeach()

  if(NANOPB_GENERATE_CPP_RELPATH)
      get_filename_component(ABS_ROOT ${NANOPB_GENERATE_CPP_RELPATH} ABSOLUTE)
  endif()
  foreach(FIL ${NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_DIR ${FIL} PATH)
    set(FIL_PATH_REL)
    if(ABS_ROOT)
      # Check that the file is under the given "RELPATH"
      string(FIND ${ABS_FIL} ${ABS_ROOT} LOC)
      if (${LOC} EQUAL 0)
        string(REPLACE "${ABS_ROOT}/" "" FIL_REL ${ABS_FIL})
        get_filename_component(FIL_PATH_REL ${FIL_REL} PATH)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL})
      endif()
    endif()
    if(NOT FIL_PATH_REL)
      set(FIL_PATH_REL ".")
    endif()

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.c")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.h")

    set(NANOPB_PLUGIN_OPTIONS)
    set(NANOPB_OPTIONS_DIRS)

    # If there an options file in the same working directory, set it as a dependency
    set(NANOPB_OPTIONS_FILE ${FIL_DIR}/${FIL_WE}.options)
    if(EXISTS ${NANOPB_OPTIONS_FILE})
      # Get directory as lookups for dependency options fail if an options
      # file is used. The options is still set as a dependency of the
      # generated source and header.
      get_filename_component(options_dir ${NANOPB_OPTIONS_FILE} DIRECTORY)
      list(APPEND NANOPB_OPTIONS_DIRS ${options_dir})
    else()
        set(NANOPB_OPTIONS_FILE)
    endif()

    # If the dependencies are options files, we need to pass the directories
    # as arguments to nanopb
    foreach(depends_file ${NANOPB_DEPENDS})
        get_filename_component(ext ${depends_file} EXT)
        if(ext STREQUAL ".options")
            get_filename_component(depends_dir ${depends_file} DIRECTORY)
            list(APPEND NANOPB_OPTIONS_DIRS ${depends_dir})
        endif()
    endforeach()

    if(NANOPB_OPTIONS_DIRS)
        list(REMOVE_DUPLICATES NANOPB_OPTIONS_DIRS)
    endif()

    foreach(options_path ${NANOPB_OPTIONS_DIRS})
        set(NANOPB_PLUGIN_OPTIONS "${NANOPB_PLUGIN_OPTIONS} -I${options_path}")
    endforeach()

    if(NANOPB_OPTIONS)
        set(NANOPB_PLUGIN_OPTIONS "${NANOPB_PLUGIN_OPTIONS} ${NANOPB_OPTIONS}")
    endif()

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.c"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS -I${GENERATOR_PATH} -I${GENERATOR_CORE_DIR}
           -I${CMAKE_CURRENT_BINARY_DIR} ${_nanopb_include_path}
           --plugin=protoc-gen-nanopb=${NANOPB_GENERATOR_PLUGIN}
           "--nanopb_out=${NANOPB_PLUGIN_OPTIONS}:${CMAKE_CURRENT_BINARY_DIR}" ${ABS_FIL}
      DEPENDS ${ABS_FIL} ${GENERATOR_CORE_PYTHON_SRC}
           ${NANOPB_OPTIONS_FILE} ${NANOPB_DEPENDS}
      COMMENT "Running C++ protocol buffer compiler using nanopb plugin on ${FIL}"
      VERBATIM )

  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} ${NANOPB_SRCS} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} ${NANOPB_HDRS} PARENT_SCOPE)

endfunction()



#
# Main.
#

# By default have NANOPB_GENERATE_CPP macro pass -I to protoc
# for each directory where a proto file is referenced.
if(NOT DEFINED NANOPB_GENERATE_CPP_APPEND_PATH)
  set(NANOPB_GENERATE_CPP_APPEND_PATH TRUE)
endif()

# Make a really good guess regarding location of NANOPB_SRC_ROOT_FOLDER
if(NOT DEFINED NANOPB_SRC_ROOT_FOLDER)
  get_filename_component(NANOPB_SRC_ROOT_FOLDER
                         ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
endif()

# Find the include directory
find_path(NANOPB_INCLUDE_DIRS
    pb.h
    PATHS ${NANOPB_SRC_ROOT_FOLDER}
)
mark_as_advanced(NANOPB_INCLUDE_DIRS)

# Find nanopb source files
set(NANOPB_SRCS)
set(NANOPB_HDRS)
list(APPEND _nanopb_srcs pb_decode.c pb_encode.c pb_common.c)
list(APPEND _nanopb_hdrs pb_decode.h pb_encode.h pb_common.h pb.h)

foreach(FIL ${_nanopb_srcs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_SRC_ROOT_FOLDER} ${NANOPB_INCLUDE_DIRS})
  list(APPEND NANOPB_SRCS "${${FIL}__nano_pb_file}")
  mark_as_advanced(${FIL}__nano_pb_file)
endforeach()

foreach(FIL ${_nanopb_hdrs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_INCLUDE_DIRS})
  mark_as_advanced(${FIL}__nano_pb_file)
  list(APPEND NANOPB_HDRS "${${FIL}__nano_pb_file}")
endforeach()

# Find the protoc Executable
find_program(PROTOBUF_PROTOC_EXECUTABLE
    NAMES protoc
    DOC "The Google Protocol Buffers Compiler"
    PATHS
    ${PROTOBUF_SRC_ROOT_FOLDER}/vsprojects/Release
    ${PROTOBUF_SRC_ROOT_FOLDER}/vsprojects/Debug
)
mark_as_advanced(PROTOBUF_PROTOC_EXECUTABLE)

# Find nanopb generator source dir
find_path(NANOPB_GENERATOR_SOURCE_DIR
    NAMES nanopb_generator.py
    DOC "nanopb generator source"
    PATHS
    ${NANOPB_SRC_ROOT_FOLDER}/generator
)
mark_as_advanced(NANOPB_GENERATOR_SOURCE_DIR)

find_package(PythonInterp REQUIRED)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NANOPB DEFAULT_MSG
  NANOPB_INCLUDE_DIRS
  NANOPB_SRCS NANOPB_HDRS
  NANOPB_GENERATOR_SOURCE_DIR
  PROTOBUF_PROTOC_EXECUTABLE
  )
