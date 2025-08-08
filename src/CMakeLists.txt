file(GLOB SRC_FILES *.cpp)
file(GLOB HEADERS_FILES *.hpp)

add_executable(find_const ${SRC_FILES} ${HEADERS_FILES})
target_link_libraries(find_const
    ${CMAKE_BINARY_DIR}/bin/libsrcsax.a
    ${CMAKE_BINARY_DIR}/bin/libsrcdispatch.a
    ${LIBXML2_LIBRARIES}
)
