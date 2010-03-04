cmake_minimum_required(VERSION 2.6)
include(CheckCXXSourceCompiles)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	if(MSVC)
		set(ICU_SUFFIX "d")
	endif()
endif()

message("-- Looking for ICU libraries")

find_library(ICU_UC icuuc${ICU_SUFFIX})
find_library(ICU_DATA icudt NAMES icudata)
find_library(ICU_I18N icuin${ICU_SUFFIX} NAMES icui18n${ICU_SUFFIX})
find_path(ICU_INCLUDE_DIR unicode/unistr.h)

message("-- Looking for Boost")
find_path(BOOST_INCLUDE_DIR boost/shared_ptr.hpp)

include_directories(${ICU_INCLUDE_DIR})

if(BOOST_INCLUDE_DIR)
	include_directories(${BOOST_INCLUDE_DIR})
endif()


check_cxx_source_compiles(
	"#include <unordered_map> 
	int main() { std::unordered_map<int,int> m; }"
	LOCALE_HAVE_STD_UNORDERED_MAP)

check_cxx_source_compiles(
	"#include <tr1/unordered_map> 
	int main() { std::tr1::unordered_map<int,int> m; }"
	LOCALE_HAVE_STD_TR1_UNORDERED_MAP)



if(CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_source_compiles(
		"#if __GNUC__ < 4
		#error
		#endif
		int main() {}"
		GCC_IS_GCC4)

	set(CXX_FLAGS "-Wall")

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

include_directories(../..)

set(BOOST_LOCALE_SRC 
	src/generator.cpp
	src/boundary.cpp
	src/timezone.cpp
	src/collator.cpp
	src/codepage.cpp
	src/conversion.cpp
	src/formatter.cpp 
	src/formatting.cpp 
	src/info.cpp 
	src/format.cpp 
	src/message.cpp 
	src/mo_lambda.cpp)

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
	target_link_libraries(boost_locale ${ICU_UC} ${ICU_DATA} ${ICU_I18N})
endif(NOT DISABLE_SHARED)


if(NOT DISABLE_STATIC)
	add_library(boost_locale-static STATIC ${BOOST_LOCALE_SRC})
	target_link_libraries(boost_locale-static ${ICU_UC} ${ICU_DATA} ${ICU_I18N})
endif(NOT DISABLE_STATIC)

set_target_properties(${BOOST_LOCALE_LIBS} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${BOOST_LOCALE_LIBS} PROPERTIES OUTPUT_NAME "boost_locale")


set(LOCALE_TESTS
		test_collate
		test_convert
		test_codepage
		test_boundary
		test_message
		test_formatting
		)
set(LOCALE_EXAMPLES 	hello
			whello
			boundary
			wboundary
			conversions
			wconversions
			collate)

foreach(TEST_TAR ${LOCALE_TESTS})
	add_executable(${TEST_TAR} test/${TEST_TAR}.cpp)
	target_link_libraries(${TEST_TAR} ${BOOST_LOCALE_LIB})
endforeach(TEST_TAR)

foreach(EX_TAR ${LOCALE_EXAMPLES})
	add_executable(${EX_TAR} examples/${EX_TAR}.cpp)
	target_link_libraries(${EX_TAR} ${BOOST_LOCALE_LIB})
endforeach(EX_TAR)

if(NOT DISABLE_SHARED)
	set_target_properties(boost_locale ${LOCALE_TESTS} ${LOCALE_EXAMPLES}
			PROPERTIES COMPILE_DEFINITIONS BOOST_LOCALE_DYN_LINK)
endif(NOT DISABLE_SHARED)


