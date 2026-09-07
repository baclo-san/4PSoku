cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
set(PROJECT_NAME 4PSoku)
project("${PROJECT_NAME}" C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")



if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC"))
	find_package(SFML)

	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	add_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-microsoft-cast")
	endif ()
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /Brepro")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /Brepro")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /Brepro")

	# Module
	add_library(
		"${PROJECT_NAME}"
		MODULE
		src/SokuMod/main.cpp
		src/SokuMod/version.rc
		src/Common/CustomPackets.hpp
		src/Common/CustomPackets.cpp
	)
	target_compile_options("${PROJECT_NAME}" PRIVATE /Zi)
	target_compile_definitions("${PROJECT_NAME}" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
	target_include_directories("${PROJECT_NAME}" PRIVATE include SokuLib/directx src/Common/)
	target_link_directories("${PROJECT_NAME}" PRIVATE lib)
	target_link_libraries(
		"${PROJECT_NAME}"
		SokuLib
		shlwapi
		d3d9
		d3dx9
	)
else ()
	find_package(SFML REQUIRED)
endif ()

# SokuLib
add_subdirectory(SokuLib)

# The relay builds either way: against SFML when it is installed, otherwise
# against the small socket shim in src/Server/MiniNet.hpp. Requiring SFML to
# build a UDP reflector turns "host your own server" into a toolchain project.
if (SFML_FOUND)
	message(STATUS "2v2Server: using SFML")
else ()
	message(STATUS "2v2Server: SFML not found, using bundled MiniNet shim")
endif ()

if (TRUE)
	if (ASAN)
		if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
			add_compile_options(/fsanitize=address)
			add_link_options(/fsanitize=address)
		else ()
			add_compile_options(-fsanitize=address)
			add_link_options(-fsanitize=address)
		endif ()
	endif ()
	# Server
	add_executable(
		2v2Server
		src/Server/main.cpp
		src/Server/Server.cpp
		src/Server/Server.hpp
		src/Common/CustomPackets.hpp
		src/Common/CustomPackets.cpp
		SokuLib/src/Packet.cpp
	)
	target_include_directories(2v2Server PRIVATE SokuLib/src src/Common)
	target_compile_definitions(2v2Server PRIVATE CURL_STATICLIB $<$<CONFIG:Debug>:_DEBUG> SOKULIB_NO_MOD)
	if (SFML_FOUND)
		target_include_directories(2v2Server PRIVATE ${SFML_INCLUDE_DIRS})
		if (WIN32)
			target_link_libraries(2v2Server ${SFML_NETWORK_LIBRARY} ${SFML_SYSTEM_LIBRARY} ws2_32)
		else()
			target_link_libraries(2v2Server ${SFML_NETWORK_LIBRARY} ${SFML_SYSTEM_LIBRARY})
		endif()
	else ()
		target_compile_definitions(2v2Server PRIVATE USE_MININET)
		if (WIN32)
			target_link_libraries(2v2Server ws2_32)
		endif()
	endif ()
endif ()