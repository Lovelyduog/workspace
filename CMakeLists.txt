cmake_minimum_required(VERSION 3.0.0)
project(demo VERSION 0.1.0)

file(GLOB SRC_LIST "*.cpp" "./vscode_test/*.cpp")
include(CTest)
enable_testing()

add_executable(demo ${SRC_LIST})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
