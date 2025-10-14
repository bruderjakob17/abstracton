cmake_minimum_required (VERSION 3.15.0)
project (abstracton)
set (CMAKE_CXX_STANDARD 20)

### Testing ### (copied from https://github.com/VeriFIT/mata/blob/devel/CMakeLists.txt)
include(CTest) # also enables testing, if it was not disabled from command line

if (BUILD_TESTING)
    include(FetchContent)
    # Catch2 library for writing tests
    FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.11.0
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
    list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
    include(Catch) # Adds Catch2::Catch2WithMain and Catch2::Catch2
    get_target_property(catch2_includes Catch2 INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(Catch2 SYSTEM INTERFACE ${catch2_includes})
endif()

find_library(LIBMATA mata REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
# TODO add_subdirectory(benchmarks)
