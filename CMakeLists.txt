cmake_minimum_required(VERSION 3.16)
project(vector LANGUAGES CXX)

# Fetching Catch2 for testing

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.0-preview4)
FetchContent_MakeAvailable(Catch2)

# Setting C++ standard variable

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Bonus flags if you don't use MSVC

if(NOT MSVC)
  add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wnull-dereference
    -Wold-style-cast
    -Wdouble-promotion
    -Wshadow)
endif()

# Target declarations

include_directories(include)

add_executable(vector_test_exec src/vector.cpp)
target_link_libraries(vector_test_exec PRIVATE Catch2::Catch2WithMain)

add_custom_target(vector_test ALL vector_test_exec)

add_custom_target(vector_valgrind_test valgrind ./vector_test_exec)
