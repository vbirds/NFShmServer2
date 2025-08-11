CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

if (UNIX)
#[[	message("nfserver makefiles process")
    execute_process(COMMAND make
  						RESULT_VARIABLE retcode
  						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})]]
endif()


