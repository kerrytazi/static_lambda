cmake_minimum_required(VERSION 3.25)

function(assign_source_group)
	foreach(_source IN ITEMS ${ARGN})
		if (IS_ABSOLUTE "${_source}")
			file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
		else()
			set(_source_rel "${_source}")
		endif()
		get_filename_component(_source_path "${_source_rel}" PATH)
		string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
		source_group("${_source_path_msvc}" FILES "${_source}")
	endforeach()
endfunction(assign_source_group)


project(static_lambda VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
	include/static_lambda/ldisasm.hpp
	src/ldisasm.cpp

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
endif()


add_library(static_lambda ${PROJECT_SOURCES})

target_include_directories(static_lambda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/static_lambda)
target_include_directories(static_lambda INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(static_lambda PRIVATE "/JMC-")

assign_source_group(${PROJECT_SOURCES})

# for debugging
add_executable(static_lambda_test test/main.cpp)
target_link_libraries(static_lambda_test static_lambda)
assign_source_group(test/main.cpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT static_lambda_test)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
