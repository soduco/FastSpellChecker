project(fsc)
cmake_minimum_required(VERSION 3.14)


add_library(fsc
  src/fsc.cpp
  include/fsc.hpp)


target_include_directories(fsc
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

target_compile_features(fsc PUBLIC cxx_std_20)
