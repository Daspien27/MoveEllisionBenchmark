cmake_minimum_required(VERSION 3.0.0)
project(MoveEllisionBenchmark VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(MoveEllisionBenchmark main.cpp "include/picobench/picobench.hpp")
target_compile_features(MoveEllisionBenchmark PUBLIC cxx_std_17)

add_executable(MinimalWorkingExample minimal.cpp "include/picobench/picobench.hpp")
target_compile_features(MinimalWorkingExample PUBLIC cxx_std_17)


add_executable(LargeTimes large.cpp "include/picobench/picobench.hpp")
target_compile_features(LargeTimes PUBLIC cxx_std_17)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
