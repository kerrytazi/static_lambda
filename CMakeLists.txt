cmake_minimum_required(VERSION 3.25)

project(static_lambda VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
	include/static_lambda/static_lambda.hpp
	include/static_lambda/detour_lambda.hpp

	src/common.hpp

	src/sysapi.hpp
	src/sysapi.cpp

	src/disasm_wrapper.hpp
	src/disasm_wrapper.cpp

	src/detours.h
	src/disasm.cpp

	src/windows_types.hpp
)

add_library(static_lambda ${PROJECT_SOURCES})

target_include_directories(static_lambda INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_source_files_properties(src/disasm.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

# cmake -DSTATIC_LAMBDA_TEST=ON -B build
option(STATIC_LAMBDA_TEST "Build static-lambda test project" OFF)

if(STATIC_LAMBDA_TEST)
	add_executable(static_lambda_test test/main.cpp)
	target_link_libraries(static_lambda_test static_lambda)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT static_lambda_test)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

