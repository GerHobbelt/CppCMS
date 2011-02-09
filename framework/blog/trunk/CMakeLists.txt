cmake_minimum_required(VERSION 2.6)
project(blog)

include_directories(${CMAKE_SOURCE_DIR})

# Options 

option(STATIC_VIEW "Build Static View instead of dynamically loaded one" OFF)


# Dependencies

find_library(LIB_BOOSTER booster)
find_library(LIB_CPPCMS cppcms)
find_library(LIB_CPPDB cppdb)
find_library(LIB_MARKDOWN markdown)
find_program(EXE_TMPL_CC cppcms_tmpl_cc)
find_program(EXE_MSGFMT msgfmt)
find_program(EXE_SQLITE3 sqlite3)

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)
find_path(INC_CPPDB cppdb/frontend.h)
find_path(INC_MARKDOWN mkdio.h)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Sources

set(VIEW_NAME contendend)

set(VIEW_TEMPLATES 
	views/blog/contendend.tmpl
	views/blog/post.tmpl
	views/blog/static_page.tmpl
	views/blog/summary.tmpl
	)

set(BLOG_SRC
	apps/blog/master.cpp
	apps/blog/post.cpp
	apps/blog/static_page.cpp
	apps/blog/blog.cpp
	apps/blog/summary.cpp
	apps/feed/feed.cpp
	apps/basic_master.cpp
	apps/main.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	)

#set(MB_LANG he)

# Actual Build

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${INC_CPPDB})
include_directories(${INC_MARKDOWN})

set(VIEW_SRC "${CMAKE_CURRENT_BINARY_DIR}/${VIEW_NAME}.cpp")

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	COMMAND ${EXE_TMPL_CC}
		${TMPL_CC_PARAMS}
		views/feed.tmpl
		-o "${CMAKE_CURRENT_BINARY_DIR}/feed.cpp"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS views/feed.tmpl)

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


add_executable(blog ${BLOG_SRC})
add_executable(blog_migrate sql/migrate.cpp)
target_link_libraries(blog ${LIB_CPPCMS})
target_link_libraries(blog ${LIB_BOOSTER})
target_link_libraries(blog ${LIB_CPPDB})
target_link_libraries(blog ${LIB_MARKDOWN})
target_link_libraries(blog_migrate ${LIB_CPPDB})

if(EXE_MSGFMT)
	set(MO_FILES)
	foreach(LANG ${MB_LANG}) 
		set(PO locale/${LANG}/LC_MESSAGES/mb.po)
		set(MO ${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/mb.mo)
		file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES")
		set(MO_FILES ${MO_FILES} ${MO})
		add_custom_command(
			OUTPUT ${MO}
			COMMAND ${EXE_MSGFMT} ${PO} -o ${MO}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${PO})
	endforeach()
	#add_custom_target(create-mo ALL DEPENDS ${MO_FILES})
else()
	message("-- msgfmt not found, translations not generatied")
endif()


