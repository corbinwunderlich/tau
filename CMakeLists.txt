cmake_minimum_required(VERSION 3.12)

file(READ "${CMAKE_CURRENT_LIST_DIR}/.version" VERSION_FILE)
string(STRIP "${VERSION_FILE}" VERSION_FILE)

project(
  tau
  VERSION ${VERSION_FILE}
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tau_core src/lexer/lexer.cpp)

target_include_directories(
  tau_core
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>
  PRIVATE src)

add_subdirectory(apps/compiler)
add_subdirectory(tests)
