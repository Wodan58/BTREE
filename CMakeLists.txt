#
#   Makefile for BTREE
#
cmake_minimum_required(VERSION 3.0)
project(btree VERSION 1.0)
set(CMAKE_BUILD_TYPE Release)
if(WIN32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage") # debug, no optimisation
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage") # enabling coverage
endif()
add_executable(gtree gtree.c rand.c)
add_executable(btree btree.c gc.c)
add_custom_target(demo01.txt ALL
		   DEPENDS gtree btree
		   COMMAND gtree i >t
		   COMMAND btree <t)
add_custom_target(demo02.txt ALL
		  DEPENDS gtree btree demo01.txt
		  COMMAND gtree s >t
		  COMMAND btree <t)
add_custom_target(demo03.txt ALL
		  DEPENDS gtree btree demo02.txt
		  COMMAND gtree d >t
		  COMMAND btree <t)
