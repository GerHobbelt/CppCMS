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
find_program(XGETTEXT xgettext)
find_program(MSGFMT msgfmt)
find_program(MSGMERGE msgmerge)

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

#  Localization


set(LOCALES he ru)

set(MO_FILES)
set(UPDATE_PO_LIST)
set(POT_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/po/wikipp.pot")

add_custom_command(
	OUTPUT ${POT_TEMPLATE}
	COMMAND 
		${XGETTEXT} 
		--keyword=translate:1,1t
		--keyword=translate:1,2,3t
		--keyword=_
		--keyword=N_
		${SRC}
		${CMAKE_CURRENT_BINARY_DIR}/view.cpp
		--output=${POT_TEMPLATE}
	DEPENDS ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/view.cpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

add_custom_target(update-po)

foreach(LOCALE ${LOCALES})
	set(MODIR "${CMAKE_CURRENT_BINARY_DIR}/locale/${LOCALE}/LC_MESSAGES")
	file(MAKE_DIRECTORY "${MODIR}")
	set(MOFILE "${MODIR}/wikipp.mo")
	set(POFILE "${CMAKE_CURRENT_SOURCE_DIR}/po/${LOCALE}.po")
	
	add_custom_command(
		OUTPUT ${MOFILE}
		COMMAND ${MSGFMT} ${POFILE} -o ${MOFILE}
		DEPENDS ${POFILE})
	
	add_custom_target(update-po-${LOCALE}
		COMMAND ${MSGMERGE} -U ${POFILE} ${CMAKE_CURRENT_SOURCE_DIR}/po/wikipp.pot 
		DEPENDS ${POT_TEMPLATE}
		)
	add_dependencies(update-po update-po-${LOCALE})

	set(MO_FILES ${MO_FILES} ${MOFILE})
	set(UPDATE_PO_LIST ${UPDATE_PO_LIST} update-po-${LOCALE})
endforeach()

add_custom_target(create-po ALL DEPENDS ${MO_FILES})

