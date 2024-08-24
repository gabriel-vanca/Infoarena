cmake_minimum_required(VERSION 3.5.0)

# Set the project name and language
project(
  Adunare
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL "%%myurl%%"
  LANGUAGES C CXX)

# include(CTest)
# enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Adunare adunare.cpp)
