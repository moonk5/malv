cmake_minimum_required(VERSION 3.0)

project(malv VERSION 1.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)
find_package(LibMPDClient REQUIRED)

include_directories(include)
link_libraries(
  stdc++fs
  Threads::Threads
  ${CURL_LIBRARIES}
  ${CURSES_LIBRARIES}
  ${LIBMPDCLIENT_LIBRARY}
)

set(SOURCES
  src/alsong.cpp
  src/tinyxml2.cpp
  src/player.cpp
  src/lyrics.cpp
  src/ui.cpp
  src/main.cpp
)

add_executable(
  ${PROJECT_NAME}
  ${SOURCES}
)


