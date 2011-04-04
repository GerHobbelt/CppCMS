cmake_minimum_required(VERSION 2.6)
include(CheckCXXSourceCompiles)
include(FindDoxygen)
enable_testing()

option(DISABLE_SHARED	"Disable shared libraries build" OFF)
option(DISABLE_STATIC	"Disable static libraries build" OFF)
option(DISABLE_ICU	"Disable ICU library/backend" OFF)
option(ENABLE_DOCS	"Enable documentation generation" OFF)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" OR APPLE)
	option(DISABLE_POSIX_BACKEND	"Disable POSIX backend" OFF)
else()
	option(DISABLE_POSIX_BACKEND	"Disable POSIX backend" ON)
endif()

if(WIN32 OR CYGWIN) 
	option(DISABLE_WINAPI_BACKEND	"Disable Win32API backend" OFF)
else()
	option(DISABLE_WINAPI_BACKEND	"Disable Win32API backend" ON)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	# stlport's std::locale is quite broken as well
	option(DISABLE_STD_BACKEND	"Disable STD backend" ON)
else()
	option(DISABLE_STD_BACKEND	"Disable STD backend" OFF)
endif()

if(DISABLE_STD_BACKEND) 
	add_definitions(-DBOOST_LOCALE_NO_STD_BACKEND)
endif()

if(DISABLE_POSIX_BACKEND) 
	add_definitions(-DBOOST_LOCALE_NO_POSIX_BACKEND)
endif()

if(DISABLE_WINAPI_BACKEND) 
	add_definitions(-DBOOST_LOCALE_NO_WINAPI_BACKEND)
endif()



if(WIN32 AND NOT CYGWIN)
	option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" ON)
else()
	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" ON)
	else()
		option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" OFF)
	endif()
endif()

if(NOT WIN32 AND NOT CYGWIN AND DISABLE_ICONV AND DISABLE_ICU)
	message(FATAL "Can't disable both ICU and iconv under POSIX platforms")
endif()


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#
# Configure proper Boost suffix of MSVC build
#

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	if(MSVC)
		set(ICU_SUFFIX "d")
	endif()
endif()

if(NOT DISABLE_ICU)
	message("-- Looking for ICU libraries")

	find_library(ICU_UC icuuc${ICU_SUFFIX})
	find_library(ICU_DATA NAMES icudt icudata)
	find_library(ICU_I18N  NAMES icuin${ICU_SUFFIX} icui18n${ICU_SUFFIX})
	find_path(ICU_INCLUDE_DIR unicode/unistr.h)
endif()

add_definitions(-DBOOST_ALL_NO_LIB)

if(ICU_UC AND ICU_DATA AND ICU_I18N AND ICU_INCLUDE_DIR)
	set(BOOST_LOCALE_HAVE_ICU ON)
	add_definitions(-DBOOST_LOCALE_WITH_ICU)
else()
	set(BOOST_LOCALE_HAVE_ICU OFF)
endif()

message("-- Looking for iconv")
if(NOT DISABLE_ICONV) 
	check_cxx_source_compiles(
		"#include <iconv.h> 
		 int main() { iconv_t v=iconv_open((char *)0,(char *)0); }"
		 LIBC_ICONV)
	if(NOT LIBC_ICONV)
		find_path(ICONV_INCLUDE_DIR iconv.h)
		find_library(ICONV_LIB iconv)
		if(ICONV_LIB AND ICONV_INCLUDE_DIR)
			set(BOOST_LOCALE_HAVE_ICONV 1)
			include_directories(${ICONV_INCLUDE_DIR})
		endif()
	else()
		set(BOOST_LOCALE_HAVE_ICONV 1)
	endif()

	if(BOOST_LOCALE_HAVE_ICONV)
		add_definitions(-DBOOST_LOCALE_WITH_ICONV)
	else()
		add_definitions(-DBOOST_LOCALE_NO_ICONV)
		message(STATUS "-- iconv not found, falling back to ICU")
	endif()
else()
	add_definitions(-DBOOST_LOCALE_NO_ICONV)
endif(NOT DISABLE_ICONV)


message("-- Looking for Boost")
find_path(BOOST_INCLUDE_DIR boost/shared_ptr.hpp)

if(BOOST_LOCALE_HAVE_ICU)
	include_directories(${ICU_INCLUDE_DIR})
endif()

if(BOOST_INCLUDE_DIR)
	include_directories(${BOOST_INCLUDE_DIR})
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_source_compiles(
		"#if __GNUC__ < 4
		#error
		#endif
		int main() {}"
		GCC_IS_GCC4)

	set(CXX_FLAGS "-Wall -Wextra")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -pthreads")
	endif()

	if(NOT GCC_IS_GCC4)
		# Uninitalized checks are bogous under gcc-3.4
		set(CXX_FLAGS "${CXX_FLAGS} -Wno-uninitialized")
	endif()

	if(WIN32)
		if(GCC_IS_GCC4) 
			# Very important, otherwise process would not start under cygwin
			set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--enable-auto-import")
		else()
			# gcc-3 does not have shared library for libstdc++ -- cause dll faitures with locale
			set(DISABLE_SHARED ON)
		endif()
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	set(CXX_FLAGS "-Wall")
elseif(MSVC)
	set(CXX_FLAGS "/EHsc /W3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	#
	# We use STL port under Sun Studio, standard library is broken
	#
	
	set(CXX_FLAGS "-library=stlport4")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -mt")
	endif()
endif()

set(CMAKE_REQUIRED_INCLUDES ${BOOST_INCLUDE_DIR})
set(CMAKE_REQUIRED_DEFINITIONS -DBOOST_ALL_NO_LIB)
set(CMAKE_REQUIRED_FLAGS ${CXX_FLAGS})

check_cxx_source_compiles(
	"#define BOOST_THREAD_NO_LIB
	#include <boost/thread.hpp>
	int main() { boost::mutex m; { boost::unique_lock<boost::mutex> l(m); } }"
	NO_BOOST_LIBRARY_NEEDED)


if(BOOST_LOCALE_HAVE_ICU OR NOT NO_BOOST_LIBRARY_NEEDED)
	if(NOT BOOST_SUFFIX AND MSVC)
		if(MSVC80)
			set(BOOST_TOOLSET "-vc80")
		elseif(MSVC90)
			set(BOOST_TOOLSET "-vc90")
		elseif(MSVC10)
			set(BOOST_TOOLSET "-vc100")
		else()
			set(BOOST_TOOLSET "")
			message("-- Undetected or unsupport version of MSVC please use -DBOOST_SUFFIX=XXX option") 
		endif()

		if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
			set(BOOST_VARIANT "-gd")
		else()
			set(BOOST_VARIANT "")
		endif()
		
		find_library(BOOST_THREAD NAMES 
				boost_thread${BOOST_TOOLSET}-mt${BOOST_VARIANT} 
				boost_thread${BOOST_TOOLSET}${BOOST_VARIANT})
	else()
		find_library(BOOST_THREAD NAMES boost_thread-mt${BOOST_SUFFIX} boost_thread${BOOST_SUFFIX})
	endif()

	if(NOT BOOST_THREAD)
		message(FATAL "Can't find boost_thread, please provide apropriate CMAKE_LIBRARY_PATH and BOOST_SUFFIX")
	endif()
endif()

if(NOT DOXYGEN_FOUND)
	if(ENABLE_DOCS)
		message("-- Doxygen not found, disaling documentation")
	endif()
	set(ENABLE_DOCS OFF)
endif()

if(ENABLE_DOCS)
	add_custom_target(docs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/locale/doc)

endif()




set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

include_directories(.)

set(BOOST_LOCALE_SRC 
	libs/locale/src/encoding/codepage.cpp

	libs/locale/src/shared/date_time.cpp
	libs/locale/src/shared/format.cpp
	libs/locale/src/shared/formatting.cpp
	libs/locale/src/shared/generator.cpp
	libs/locale/src/shared/ids.cpp
	libs/locale/src/shared/localization_backend.cpp
	libs/locale/src/shared/message.cpp
	libs/locale/src/shared/mo_lambda.cpp

	libs/locale/src/util/info.cpp
	libs/locale/src/util/locale_data.cpp
	libs/locale/src/util/default_locale.cpp
	libs/locale/src/util/codecvt_converter.cpp
	
	)

if(NOT DISABLE_STD_BACKEND)
	set(BOOST_LOCALE_SRC ${BOOST_LOCALE_SRC}
		libs/locale/src/std/codecvt.cpp
		libs/locale/src/std/collate.cpp
		libs/locale/src/std/converter.cpp
		libs/locale/src/std/numeric.cpp
		libs/locale/src/std/std_backend.cpp
	)
endif()

if(NOT DISABLE_POSIX_BACKEND)
	set(BOOST_LOCALE_SRC ${BOOST_LOCALE_SRC}
		libs/locale/src/posix/collate.cpp
		libs/locale/src/posix/converter.cpp
		libs/locale/src/posix/numeric.cpp
		libs/locale/src/posix/codecvt.cpp
		libs/locale/src/posix/posix_backend.cpp
	)
endif()

if(NOT DISABLE_WINAPI_BACKEND)
	set(BOOST_LOCALE_SRC ${BOOST_LOCALE_SRC}
		libs/locale/src/win32/collate.cpp
		libs/locale/src/win32/converter.cpp
		libs/locale/src/win32/numeric.cpp
		libs/locale/src/win32/win_backend.cpp
	)
endif()

if(NOT DISABLE_WINAPI_BACKEND OR NOT DISABLE_POSIX_BACKEND OR NOT DISABLE_STD_BACKEND)
	set(BOOST_LOCALE_SRC ${BOOST_LOCALE_SRC}
		libs/locale/src/util/gregorian.cpp
	)

endif()

if(WIN32 OR CYGWIN)
    if(NOT DISABLE_WINAPI_BACKEND OR NOT DISABLE_STD_BACKEND)
        set(BOOST_LOCALE_SRC ${BOOST_LOCALE_SRC}
            libs/locale/src/win32/lcid.cpp
        )
    endif()
endif()


if(BOOST_LOCALE_HAVE_ICU)
	set(BOOST_LOCALE_SRC 
		${BOOST_LOCALE_SRC}
		libs/locale/src/icu/boundary.cpp
		libs/locale/src/icu/codecvt.cpp
		libs/locale/src/icu/collator.cpp
		libs/locale/src/icu/conversion.cpp
		libs/locale/src/icu/date_time.cpp
		libs/locale/src/icu/formatter.cpp
		libs/locale/src/icu/icu_backend.cpp
		libs/locale/src/icu/numeric.cpp
		libs/locale/src/icu/time_zone.cpp)
endif()


if(DISABLE_SHARED AND DISABLE_STATIC)
	message(FATAL " Can't build with both DISABLE_SHARED AND DISABLE_STATIC")
endif(DISABLE_SHARED AND DISABLE_STATIC)

if(DISABLE_SHARED)
	set(BOOST_LOCALE_LIBS boost_locale-static)
	set(BOOST_LOCALE_LIB boost_locale-static)
elseif(DISABLE_STATIC)
	set(BOOST_LOCALE_LIBS boost_locale)
	set(BOOST_LOCALE_LIB boost_locale)
else(DISABLE_STATIC)
	set(BOOST_LOCALE_LIBS boost_locale; boost_locale-static)
	set(BOOST_LOCALE_LIB boost_locale)
endif(DISABLE_SHARED)

if(NOT DISABLE_SHARED)
	add_library(boost_locale SHARED ${BOOST_LOCALE_SRC})
	if(BOOST_LOCALE_HAVE_ICU)
		target_link_libraries(boost_locale ${ICU_UC} ${ICU_DATA} ${ICU_I18N})
	endif()
	if(ICONV_LIB)
		target_link_libraries(boost_locale ${ICONV_LIB})
	endif()
	if(BOOST_THREAD)
		target_link_libraries(boost_locale ${BOOST_THREAD})
	endif()
endif(NOT DISABLE_SHARED)


if(NOT DISABLE_STATIC)
	add_library(boost_locale-static STATIC ${BOOST_LOCALE_SRC})
	if(BOOST_LOCALE_HAVE_ICU)
		target_link_libraries(boost_locale-static ${ICU_UC} ${ICU_DATA} ${ICU_I18N})
	endif()
	if(ICONV_LIB)
		target_link_libraries(boost_locale-static ${ICONV_LIB})
	endif()
	if(BOOST_THREAD)
		target_link_libraries(boost_locale-static ${BOOST_THREAD})
	endif()
endif(NOT DISABLE_STATIC)

set_target_properties(${BOOST_LOCALE_LIBS} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${BOOST_LOCALE_LIBS} PROPERTIES OUTPUT_NAME "boost_locale")



set(LOCALE_TESTS
		test_codepage
		test_message
		test_ios_prop
		test_codepage_converter
		test_date_time
		)

if(NOT DISABLE_STD_BACKEND)
	set(LOCALE_TESTS ${LOCALE_TESTS}
		test_std_convert
		test_std_formatting
		test_std_collate
		)
endif()

if(NOT DISABLE_POSIX_BACKEND) 
	set(LOCALE_TESTS ${LOCALE_TESTS}
		test_posix_collate
		test_posix_convert
		test_posix_formatting
		)
endif()

if(NOT DISABLE_WINAPI_BACKEND) 
	set(LOCALE_TESTS ${LOCALE_TESTS}
		test_winapi_formatting
		test_winapi_collate
		test_winapi_convert
		)
endif()

if(BOOST_LOCALE_HAVE_ICU)
	set(LOCALE_TESTS
			${LOCALE_TESTS}
			test_collate
			test_convert
			test_boundary
			test_formatting
			test_generator
			test_icu_vs_os_timezone
			)
endif()



set(LOCALE_EXAMPLES 	hello
			whello
			boundary
			wboundary
			conversions
			wconversions
			collate
			calendar
			)

set(PERF_TESTS		perf_collate
			perf_convert
			perf_format)


foreach(TEST_TAR ${LOCALE_TESTS})
	add_executable(${TEST_TAR} libs/locale/test/${TEST_TAR}.cpp)
	add_test(${TEST_TAR} ${TEST_TAR} "${CMAKE_CURRENT_SOURCE_DIR}/libs/locale/test")
	target_link_libraries(${TEST_TAR} ${BOOST_LOCALE_LIB})
endforeach(TEST_TAR)

foreach(EX_TAR ${LOCALE_EXAMPLES})
	add_executable(${EX_TAR} libs/locale/examples/${EX_TAR}.cpp)
	target_link_libraries(${EX_TAR} ${BOOST_LOCALE_LIB})
endforeach(EX_TAR)

foreach(EX_TAR ${PERF_TESTS})
	add_executable(${EX_TAR} libs/locale/performance/${EX_TAR}.cpp)
	target_link_libraries(${EX_TAR} ${BOOST_LOCALE_LIB})
endforeach(EX_TAR)



if(NOT DISABLE_SHARED)
	set_target_properties(boost_locale ${LOCALE_TESTS} ${LOCALE_EXAMPLES} ${PERF_TESTS}
			PROPERTIES COMPILE_DEFINITIONS BOOST_LOCALE_DYN_LINK)
endif(NOT DISABLE_SHARED)

if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(boost_locale-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

if(GCC_IS_GCC4)
	if(BOOST_LOCALE_HAVE_ICU)
		# Workaround huge compilation times
		set_target_properties(test_formatting PROPERTIES COMPILE_FLAGS "-O0")
	endif()
endif()

install(TARGETS ${BOOST_LOCALE_LIBS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

install(DIRECTORY boost DESTINATION include
        PATTERN ".svn" EXCLUDE)

if(ENABLE_DOCS)
	install(DIRECTORY libs/locale/doc/html DESTINATION share/doc/boost_locale
	        PATTERN ".svn" EXCLUDE)
endif()

install(DIRECTORY libs/locale/examples DESTINATION share/doc/boost_locale
        PATTERN ".svn" EXCLUDE)
	
