project(FastSpellChecker VERSION 0.0.2)
cmake_minimum_required(VERSION 3.14)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra)
endif()
add_subdirectory(libfsc)


include(CTest)
enable_testing()
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
