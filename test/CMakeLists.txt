cmake_minimum_required(VERSION 3.25 )
cmake_policy(VERSION 3.25)
set(CMAKE_CXX_STANDARD 17)

project ( itch_tests )

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable( itch_tests 
    simple.cpp
)

target_include_directories(itch_tests PRIVATE 
    ../include
)

target_link_libraries(itch_tests 
    GTest::gtest
    GTest::gtest_main
)

