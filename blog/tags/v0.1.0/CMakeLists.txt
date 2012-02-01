cmake_minimum_required(VERSION 2.6)
project(cppcms_blog)

include_directories(${CMAKE_SOURCE_DIR})
include(FindPkgConfig)
include(CPack)



# Options 

option(STATIC_VIEW "Build Static View instead of dynamically loaded one" OFF)


# Dependencies

find_library(LIB_BOOSTER booster)
find_library(LIB_CPPCMS cppcms)
find_library(LIB_CPPDB cppdb)
find_library(LIB_MARKDOWN markdown)
find_program(EXE_TMPL_CC cppcms_tmpl_cc)
find_program(EXE_MSGFMT msgfmt)
find_program(EXE_MSGMERGE msgmerge)
find_program(XGETTEXT xgettext)
find_program(EXE_SQLITE3 sqlite3)

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)
find_path(INC_CPPDB cppdb/frontend.h)
find_path(INC_MARKDOWN mkdio.h)


pkg_search_module(IM REQUIRED ImageMagick++ GraphicsMagick++)
include_directories(${IM_INCLUDE_DIRS})


if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(WIN32 OR CYGWIN)
	# We use shared libraries...
	add_definitions(-DDLL_EXPORT)
endif()

# Sources

set(SKINS contendend orangesky)

set(ADMIN_TEMPLATES 
	views/admin/master.tmpl
	views/admin/summary.tmpl
	views/admin/users.tmpl
	views/admin/page.tmpl
	views/admin/post.tmpl
	views/admin/caching.tmpl
	views/admin/options.tmpl
	views/admin/comment.tmpl
	views/admin/categories.tmpl
	views/admin/config.tmpl
	)


set(BLOG_SRC
	apps/blog/master.cpp
	apps/blog/post.cpp
	apps/blog/page.cpp
	apps/blog/blog.cpp
	apps/blog/summary.cpp
	apps/feed/feed.cpp
	apps/basic_master.cpp
	apps/captcha.cpp
	apps/config.cpp
	apps/main.cpp
	apps/admin/admin.cpp
	apps/admin/users.cpp
	apps/admin/summary.cpp
	apps/admin/page.cpp
	apps/admin/post.cpp
	apps/admin/caching.cpp
	apps/admin/options.cpp
	apps/admin/comment.cpp
	apps/admin/categories.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	"${CMAKE_CURRENT_BINARY_DIR}/admin.cpp"
	)


# Actual Build

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${INC_CPPDB})
include_directories(${INC_MARKDOWN})


add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/admin.cpp"
	COMMAND ${EXE_TMPL_CC}
		${TMPL_CC_PARAMS}
		${ADMIN_TEMPLATES}
		-o "${CMAKE_CURRENT_BINARY_DIR}/admin.cpp"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${ADMIN_TEMPLATES})


add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	COMMAND ${EXE_TMPL_CC}
		${TMPL_CC_PARAMS}
		views/feed.tmpl
		-o "${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS views/feed.tmpl)


foreach(SKIN ${SKINS})
	set(VIEW_NAME ${SKIN})
	set(VIEW_TEMPLATES 
		views/blog/basic_master.tmpl
		views/blog/skins/${SKIN}.tmpl
		views/blog/post.tmpl
		views/blog/page.tmpl
		views/blog/summary.tmpl
		)

	set(VIEW_SRC "${CMAKE_CURRENT_BINARY_DIR}/${VIEW_NAME}.cpp")

	add_custom_command(
		OUTPUT ${VIEW_SRC}
		COMMAND ${EXE_TMPL_CC}
			-s "${VIEW_NAME}"
			${TMPL_CC_PARAMS}
			-o ${VIEW_SRC}
			${VIEW_TEMPLATES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${VIEW_TEMPLATES})


	if(STATIC_VIEW)
		set(BLOG_SRC ${BLOG_SRC} ${VIEW_SRC})
	else()
		add_library(${VIEW_NAME} SHARED ${VIEW_SRC})
		target_link_libraries(${VIEW_NAME} ${LIB_CPPCMS})
		target_link_libraries(${VIEW_NAME} ${LIB_BOOSTER})
	endif()
endforeach()


add_executable(cppblog ${BLOG_SRC})
add_executable(cppblog_migrate sql/migrate.cpp)
target_link_libraries(cppblog ${LIB_CPPCMS})
target_link_libraries(cppblog ${LIB_BOOSTER})
target_link_libraries(cppblog ${LIB_CPPDB})
target_link_libraries(cppblog ${LIB_MARKDOWN})
target_link_libraries(cppblog ${IM_LIBRARIES})
target_link_libraries(cppblog_migrate ${LIB_CPPDB})

if(NOT EXE_MSGFMT)
	message("-- msgfmt not found, translations not generatied")
endif()

#  Localization

set(LOCALES he zh)

set(MO_FILES)
set(UPDATE_PO_LIST)
set(POT_TEMPLATE po/blog.pot)

file(GLOB_RECURSE GETTEXT_SRC 
	apps/*.h 
	apps/*.cpp 
	data/*.h 
	data/*.cpp)


set(SKINCPP)
foreach(SKIN ${SKINS})
	set(SKINCPP ${SKINCPP} "${CMAKE_CURRENT_BINARY_DIR}/${SKIN}.cpp")
endforeach()

set(EXTRACT_SRC ${GETTEXT_SRC} ${CMAKE_CURRENT_BINARY_DIR}/feed.cpp ${CMAKE_CURRENT_BINARY_DIR}/admin.cpp ${SKINCPP})

add_custom_command(
	OUTPUT ${POT_TEMPLATE}
	COMMAND 
		${XGETTEXT} 
		--keyword=translate:1,1t
		--keyword=translate:1c,2,2t 
		--keyword=translate:1,2,3t
		--keyword=translate:1c,2,3,4t   
		--keyword=gettext:1
		--keyword=pgettext:1c,2               
		--keyword=ngettext:1,2
		--keyword=npgettext:1c,2,3 
		--keyword=_
		--keyword=N_
		${EXTRACT_SRC}
		--output=${POT_TEMPLATE}
	DEPENDS ${EXTRACT_SRC}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

add_custom_target(update-po)

foreach(LOCALE ${LOCALES})
	set(MODIR "${CMAKE_CURRENT_BINARY_DIR}/locale/${LOCALE}/LC_MESSAGES")
	file(MAKE_DIRECTORY "${MODIR}")
	set(MOFILE "${MODIR}/cppblog.mo")
	set(POFILE "${CMAKE_CURRENT_SOURCE_DIR}/po/${LOCALE}.po")
	
	add_custom_command(
		OUTPUT ${MOFILE}
		COMMAND ${EXE_MSGFMT} ${POFILE} -o ${MOFILE}
		DEPENDS ${POFILE})
	
	add_custom_target(update-po-${LOCALE}
		COMMAND ${EXE_MSGMERGE} -U ${POFILE} ${POT_TEMPLATE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${POT_TEMPLATE}
		)
	add_dependencies(update-po update-po-${LOCALE})

	set(MO_FILES ${MO_FILES} ${MOFILE})
	set(UPDATE_PO_LIST ${UPDATE_PO_LIST} update-po-${LOCALE})
endforeach()

add_custom_target(create-po ALL DEPENDS ${MO_FILES})

install(TARGETS cppblog cppblog_migrate
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib/cppblog
	ARCHIVE DESTINATION lib/cppblog)

if(NOT STATIC_VIEW)
	install(TARGETS ${SKINS}
		RUNTIME DESTINATION bin/cppblog
		LIBRARY DESTINATION lib/cppblog
		ARCHIVE DESTINATION lib/cppblog)
endif()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale 
	DESTINATION share)
install(DIRECTORY media/ 
	DESTINATION share/cppblog/media
        PATTERN ".svn" EXCLUDE)

install(FILES	sql/sqlite3.sql 
		sql/mysql.sql
		sql/postgresql.sql
		sql/upgrade-1-to-2.sql
		sql/upgrade-2-to-3.sql
	DESTINATION share/cppblog/sql)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sample_config.js DESTINATION share/cppblog)

