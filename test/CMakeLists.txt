file(GLOB TEST_FILES *.cpp)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/input_file)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input_file/input.xml
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/input_file)

add_executable(test_runner ${TEST_FILES})
target_link_libraries(test_runner
    GTest::gtest
    GTest::gtest_main
    pthread
    ${CMAKE_BINARY_DIR}/bin/libsrcsax.a
    ${CMAKE_BINARY_DIR}/bin/libsrcdispatch.a
    ${LIBXML2_LIBRARIES}
)

add_test(NAME AllTests COMMAND test_runner)
