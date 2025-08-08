cmake_minimum_required(VERSION 3.14)
project(srcFCS)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

# Find required packages
find_package(LibXml2 REQUIRED)
find_package(GTest REQUIRED)

# Include directories
include_directories(
    ${LIBXML2_INCLUDE_DIR}
)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(srcDispatch)

include_directories(${DISPATCH_INCLUDE_DIR} src test)

add_subdirectory(src)
add_subdirectory(test)