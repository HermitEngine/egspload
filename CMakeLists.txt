cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(egspload)
set(EGSP_VERSION_MAJOR 0)
set(EGSP_VERSION_MINOR 0)
set(EGSP_VERSION_REV   0)

option(EGSP_BUILD_TESTS "Build Unit Tests" OFF)
option(EGSP_JSON "Egspload Json Support" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

set(SRC
	src/egsploader.c
	)

set(LIB_SRC
	src/egsplib.h
	src/egsplib.c
	)

set(TEST_SRC
	test/egsptest.c
	test/egsptest.egsp
	test/egspload.h
	)

add_library(egspload ${LIB_SRC})
add_executable(egsploader ${SRC})
set_target_properties(egspload PROPERTIES FOLDER "Egspload")
set_target_properties(egsploader PROPERTIES FOLDER "Egspload")

if(EGSP_BUILD_TESTS)
	add_custom_command(TARGET egsploader
		POST_BUILD
		COMMAND egsploader egsptest.egsp
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	)
	add_executable(egsptest ${TEST_SRC})
	add_dependencies(egsptest egsploader)
	target_link_libraries(egsptest egspload)
	include_directories(src)
	set_target_properties(egsptest PROPERTIES FOLDER "Egspload")
endif(EGSP_BUILD_TESTS)

if(EGSP_JSON)
	add_definitions(-DEGSP_JSON)
endif(EGSP_JSON)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
