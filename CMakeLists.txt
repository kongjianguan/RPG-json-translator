cmake_minimum_required(VERSION 3.0)
project(RPGMaker-Json-Translator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

add_executable(RPGMaker-Json-Translator src/tran.cpp)

target_link_libraries(RPGMaker-Json-Translator CURL::libcurl )
target_link_libraries(RPGMaker-Json-Translator crypto++)