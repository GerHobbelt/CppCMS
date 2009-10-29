cmake_minimum_required(VERSION 2.4)

find_library(ICU_UC icuuc)
find_library(ICU_DATA icudata NAMES icudt)
find_library(ICU_I18N icui18n NAMES icuin)

if(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)
	message(FATAL " Failed to find icu")
endif(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)

add_definitions(-DBOOST_LOCALE_DYN_LINK)

include_directories(../..)

add_library(boost_locale SHARED 
	src/collator.cpp
	src/converter.cpp
	src/formatter.cpp 
	src/formatting.cpp 
	src/info.cpp 
	src/format.cpp 
	src/message.cpp 
	src/mo_lambda.cpp)

add_executable(test src/test.cpp)

target_link_libraries(boost_locale ${ICU_UC})
target_link_libraries(boost_locale ${ICU_DATA})
target_link_libraries(boost_locale ${ICU_I18N})

target_link_libraries(test boost_locale)

