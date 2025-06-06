cmake_minimum_required(VERSION 3.25)

project(static_lambda VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
	include/static_lambda/ldisasm.hpp
	src/ldisasm.cpp
	src/disasm.cpp

	include/static_lambda/sysapi.hpp
	src/sysapi.cpp

	include/static_lambda/common.hpp
	include/static_lambda/static_lambda.hpp
	include/static_lambda/detour_lambda.hpp
)

if(WIN32)
	enable_language(ASM_MASM)
	list(APPEND PROJECT_SOURCES
		include/static_lambda/slwinapi.hpp
		src/slwinapi.asm
	)

	if(NOT MSVC)
		message(FATAL_ERROR "Only msvc is supported on Windows")
	endif()
elseif(UNIX)
	list(APPEND PROJECT_SOURCES
		include/static_lambda/slunixapi.hpp
	)
endif()


add_library(static_lambda ${PROJECT_SOURCES})

target_include_directories(static_lambda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/static_lambda)
target_include_directories(static_lambda INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
	target_compile_options(static_lambda PRIVATE "/JMC-")
endif()

# cmake -DCCALL_TEST=ON -B build
option(STATIC_LAMBDA_TEST "Build static-lambda test project" OFF)

if(STATIC_LAMBDA_TEST)
	add_executable(static_lambda_test test/main.cpp)
	target_link_libraries(static_lambda_test static_lambda)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT static_lambda_test)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		target_compile_options(static_lambda_test PUBLIC
			$<$<CONFIG:Debug>:-O0 -g>
			$<$<CONFIG:MinSizeRel>:-Os -s>
			$<$<CONFIG:Release>:-O2 -s>
			$<$<CONFIG:RelWithDebInfo>:-O2 -g>
		)
	endif()
endif()
