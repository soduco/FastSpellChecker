find_package(GTest REQUIRED)


add_executable(tests tests.cpp tests_data.cpp)
target_link_libraries(tests GTest::GTest GTest::Main fsc)

add_test(UTtests tests)
