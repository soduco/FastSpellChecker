find_package(GTest REQUIRED)


add_executable(tests tests.cpp tests_data.cpp)
target_link_libraries(tests GTest::GTest GTest::Main dictionary)

add_test(UTtests tests)
