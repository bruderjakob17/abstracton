find_package(jsoncpp REQUIRED)

add_library(parsers STATIC
    DodoParser.cpp
)

target_include_directories(parsers PUBLIC mata)
target_include_directories(parsers PUBLIC ${PROJECT_SOURCE_DIR}/benchmarks)
target_link_libraries(parsers jsoncpp)

add_executable(solve_dodo solve_dodo.cpp)
target_link_libraries(solve_dodo parsers)
target_link_libraries(solve_dodo mata)
