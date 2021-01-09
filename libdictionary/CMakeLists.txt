project(libdictionary)
cmake_minimum_required(VERSION 3.14)


add_library(dictionary
  src/dictionary.cpp
  include/dictionary.hpp)


target_include_directories(dictionary
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

target_compile_features(dictionary PUBLIC cxx_std_20)

