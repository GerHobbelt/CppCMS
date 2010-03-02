cmake_minimum_required(VERSION 2.4)

find_library(ICU_UC icuuc)
find_library(ICU_DATA icudata NAMES icudt)
find_library(ICU_I18N icui18n NAMES icuin)

if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX AND NOT DISABLE_SHARED)
	include(CheckCXXSourceCompiles)
	check_cxx_source_compiles(
		"#if __GNUC__ < 4
		#error
		#endif
		int main() {return 0;}"
		GCC_IS_GCC4)
	if(NOT GCC_IS_GCC4)
		message("GCC version is below gcc4, thus disabling shared library")
		set(DISABLE_SHARED 1)
	endif(NOT GCC_IS_GCC4)
endif(WIN32 AND CMAKE_COMPILER_IS_GNUCXX AND NOT DISABLE_SHARED)


if(GCC_IS_GCC4 AND WIN32)
	set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--enable-auto-import")
endif(GCC_IS_GCC4 AND WIN32)
	

if(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)
	message(FATAL " Failed to find icu")
endif(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)

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


