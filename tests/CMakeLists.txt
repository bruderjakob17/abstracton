add_executable(tests
    inductive_sets.cpp
    preach.cpp
    mata_extensions.cpp
    interpretations.cpp
    dodo_parser.cpp
    utils/utils.cpp
    bddlike/bddlike_nft.cpp
    bddlike/interpretations.cpp
    bddlike/inductive_sets.cpp
    bddlike/preach.cpp
)

target_link_libraries(tests PRIVATE abstracton parsers Catch2::Catch2WithMain)
target_compile_definitions(tests PRIVATE ABSTRACTON_BENCHMARKS_DIR="${PROJECT_SOURCE_DIR}/benchmarks")

catch_discover_tests(tests)
