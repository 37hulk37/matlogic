cmake_minimum_required(VERSION 3.10)
find_library(buddyLib buddy HINTS ./lib REQUIRED)
project(matlogic LANGUAGES CXX C)
set(target matlogic)
set(CMAKE_CXX_STANDARD 23)
option(BUILD_TEST OFF)
set(SOURCE_LIST 
  include/bdd.h
  src/BDDHelper.hpp
  src/BDDHelper.cpp
  src/BDDFormulaBuilder.hpp
  src/BDDFormulaBuilder.cpp
  src/Conditions.hpp
  src/Conditions.cpp
  src/PrintHelper.hpp
  src/PrintHelper.cpp
  src/config.cpp src/config.h
  include/magic_enum.h
)

add_executable(${target}
  src/main.cpp
  ${SOURCE_LIST}
)
# if (CMAKE_BUILD_TYPE STREQUAL "Release")
#   message("Type set to Release. Enabling super optimization")
#   target_compile_options(${target} PUBLIC -O3)
# endif()
set_target_properties(${target} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${target} PUBLIC ${buddyLib})
target_include_directories(${target} PUBLIC include)
if (BUILD_TEST)
  enable_testing()
  set(test_target test_target)
  add_executable(${test_target}
    src/mainTEST.cpp
          ${SOURCE_LIST}
  )
  set_target_properties(${test_target} PROPERTIES CXX_STANDARD 23)
  target_compile_definitions(${test_target} PRIVATE GTEST_TESTING)
  find_package(GTest REQUIRED)
  target_link_libraries(${test_target} PUBLIC ${buddyLib} gtest gtest_main)
  target_include_directories(${test_target} PUBLIC include)
endif()
