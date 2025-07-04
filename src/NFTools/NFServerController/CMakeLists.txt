CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NFServerController)

AUX_SOURCE_DIRECTORY(./ SRC)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt  pthread)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libvcruntime.lib msvcrt.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES 
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Release"
	)
if(UNIX)
	ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND mv ${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Release/${PROJECT_NAME} ${CMAKE_NFSHM_SOURCE_DIR}/tools/${PROJECT_NAME}
	)
endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES 
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Debug"
	)
elseif (CMAKE_BUILD_TYPE STREQUAL "DynamicRelease")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Dynamic_Release"
	)

	ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND mv ${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Dynamic_Release/${PROJECT_NAME} ${CMAKE_NFSHM_SOURCE_DIR}/tools/${PROJECT_NAME}
	)

elseif(CMAKE_BUILD_TYPE STREQUAL "DynamicDebug")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} 
		PROPERTIES 
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Dynamic_Debug"
	)
endif()




