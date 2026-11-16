cmake_minimum_required(VERSION 3.20)
project(sfp_driver_tests LANGUAGES C CXX)

include(CTest)
include(FetchContent)

# Keep the fetched dependency reproducible and build only the framework library.
set(TESTS OFF CACHE BOOL "Disable CppUTest's own test suite" FORCE)
set(EXTENSIONS OFF CACHE BOOL "Disable CppUTest extensions" FORCE)
set(CPPUTEST_FLAGS OFF CACHE BOOL "Use the parent project's compiler flags" FORCE)

FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(CppUTest)

add_executable(sfp_driver_tests
    test_sfp_driver.cpp
    ../../Appli/Src/drivers/sfp/sfp_driver.c
)

target_compile_features(sfp_driver_tests PRIVATE c_std_11 cxx_std_11)
target_include_directories(sfp_driver_tests PRIVATE
    include
    ../../Appli/Inc
    ../../Appli/Src/drivers/sfp
)
target_link_libraries(sfp_driver_tests PRIVATE CppUTest)

if(UNIX)
    target_link_libraries(sfp_driver_tests PRIVATE m)
endif()

add_test(NAME sfp_driver_tests COMMAND sfp_driver_tests)
