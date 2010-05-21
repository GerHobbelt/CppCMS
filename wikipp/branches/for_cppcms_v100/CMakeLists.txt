cmake_minimum_required(VERSION 2.6)
project(wikipp)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(USE_STATIC_VIEW "Compile view statically into wikipp" OFF)



find_library(CPPCMS cppcms)
find_library(BOOSTER booster)
find_library(DBIXX dbixx)

find_path(CPPCMS_INC cppcms/application.h)
find_path(BOOSTER_INC booster/shared_ptr.h)
find_path(DBIXX_INC dbixx/dbixx.h)

include_directories(${CPPCMS_INC})
include_directories(${BOOSTER_INC})
include_directories(${DBIXX_INC})
include_directories(.)

find_program(TMPLCC cppcms_tmpl_cc)

if(WIN32)
	add_definitions(-DDLL_EXPORT)
endif()


set(TEMPLATES 
	${CMAKE_CURRENT_SOURCE_DIR}/templates/main.tmpl
	${CMAKE_CURRENT_SOURCE_DIR}/templates/page.tmpl
	${CMAKE_CURRENT_SOURCE_DIR}/templates/hist.tmpl
	${CMAKE_CURRENT_SOURCE_DIR}/templates/admin.tmpl
	${CMAKE_CURRENT_SOURCE_DIR}/templates/toc.tmpl
)

set(SRC 
	index.cpp
	main.cpp
	master.cpp
	options.cpp
	page.cpp
	users.cpp
	wiki.cpp
	cxxmarkdown/docheader.c
	cxxmarkdown/dumptree.c
	cxxmarkdown/generate.c
	cxxmarkdown/markdown.c
	cxxmarkdown/markdowncxx.cpp
	cxxmarkdown/mkdio.c
	cxxmarkdown/resource.c  
	cxxmarkdown/version.c
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/view.cpp
	COMMAND ${TMPLCC}
		-d wikipp
		-o ${CMAKE_CURRENT_BINARY_DIR}/view.cpp 
		${TEMPLATES}
	DEPENDS ${TEMPLATES})


if(USE_STATIC_VIEW)
	add_executable(wikipp ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/view.cpp)
else()
	add_executable(wikipp ${SRC})
	add_library(view SHARED ${CMAKE_CURRENT_BINARY_DIR}/view.cpp)
	target_link_libraries(view ${BOOSTER} ${CPPCMS})
endif()


target_link_libraries(wikipp ${BOOSTER} ${CPPCMS} ${DBIXX})

