CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NFServerStatic)

AUX_SOURCE_DIRECTORY(. SRC)
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
elseif (CMAKE_BUILD_TYPE STREQUAL "DynamicRelease" OR CMAKE_BUILD_TYPE STREQUAL "DynamicDebug")
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFCore SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFPluginManager SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFPluginModule SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFKernelMessage SRC)
    AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFObjCommon SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFPluginModule/NFJson2PB SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/src/NFrame/NFComm/NFPluginModule/NFProto SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/lpeg SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/lua SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/luacjson SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/protoc-gen-lua SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/lua_protobuf SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/luafilesystem SRC)
	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/LuaBind/lua-fmt SRC)

	if(WIN32)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/sigar/win32 SRC)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/hiredis/hiredis_win/hiredis SRC)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/hwinfo/source/win SRC)
	else(UNIX)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/sigar/linux SRC)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/hiredis/hiredis_linux/hiredis SRC)
		AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/hwinfo/source/linux SRC)
	endif()

	AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/lzf SRC)
endif()

AUX_SOURCE_DIRECTORY(${CMAKE_NFSHM_SOURCE_DIR}/thirdparty/common/lzf SRC)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt  pthread tirpc libprotobuf.a  libprotobuf-nanopb.a)
	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Release"
	)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrt.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib winmm.lib
			NFKernelPlugin NFNetPlugin NFShmPlugin NFMemPlugin NFDBPlugin NFTutorialPlugin #frame
			NFServerCommonPlugin NFDescStorePlugin NFCenterServerPlugin NFGameServerPlugin #server
			NFLogicServerPlugin NFLoginServerPlugin NFMasterServerPlugin NFProxyAgentServerPlugin #server
			NFProxyServerPlugin	NFProxyClientPlugin NFRouteAgentServerPlugin NFRouteServerPlugin #server
			NFCityServerPlugin NFMatchServerPlugin NFNavMeshServerPlugin NFOnlineServerPlugin #server
			NFSnsServerPlugin NFRouteServerPlugin NFStoreServerPlugin NFWebServerPlugin NFWorldServerPlugin NFLuaScriptPlugin #server
	)
	if(MSVC)
		SET_TARGET_PROPERTIES(${PROJECT_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin"
		)
	else()
		SET_TARGET_PROPERTIES(${PROJECT_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Release"
		)
	endif()
endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread tirpc libprotobuf.a  libprotobuf-nanopb.a)
	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Debug"
	)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib winmm.lib
			NFKernelPlugin NFNetPlugin NFShmPlugin NFMemPlugin NFDBPlugin NFTutorialPlugin #frame
			NFServerCommonPlugin NFDescStorePlugin NFCenterServerPlugin NFGameServerPlugin #server
			NFLogicServerPlugin NFLoginServerPlugin NFMasterServerPlugin NFProxyAgentServerPlugin #server
			NFProxyServerPlugin	NFProxyClientPlugin NFRouteAgentServerPlugin NFRouteServerPlugin #server
			NFCityServerPlugin NFMatchServerPlugin NFNavMeshServerPlugin NFOnlineServerPlugin #server
			NFSnsServerPlugin NFRouteServerPlugin NFStoreServerPlugin NFWebServerPlugin NFWorldServerPlugin NFLuaScriptPlugin #server
	)
	if(MSVC)
		SET_TARGET_PROPERTIES(${PROJECT_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Debug"
		)
	else()
		SET_TARGET_PROPERTIES(${PROJECT_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Debug"
		)
	endif()
endif()
elseif (CMAKE_BUILD_TYPE STREQUAL "DynamicRelease")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread tirpc libprotobuf.a libprotobuf-nanopb.a)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

	SET_TARGET_PROPERTIES(${PROJECT_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Dynamic_Release"
	)
elseif(CMAKE_BUILD_TYPE STREQUAL "DynamicDebug")
if(UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libprotobuf.a resolv dl rt pthread tirpc libprotobuf.a libprotobuf-nanopb.a)
else(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} msvcrtd.lib ws2_32.lib version.lib netapi32.lib Dbghelp.lib)
endif()
	#TARGET_LINK_LIBRARIES(${PROJECT_NAME} resolv dl rt pthread libprotobuf.a)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} 
		PROPERTIES 
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_NFSHM_SOURCE_DIR}/Install/Bin/Dynamic_Debug"
	)
endif()




