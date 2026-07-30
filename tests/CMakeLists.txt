add_executable(tests
    inductive_sets.cpp
    preach.cpp
    mata_extensions.cpp
    interpretations.cpp
    utils/utils.cpp
    bddlike/bddlike_nft.cpp
    bddlike/interpretations.cpp
    bddlike/inductive_sets.cpp
)

target_link_libraries(tests PRIVATE abstracton Catch2::Catch2WithMain)

catch_discover_tests(tests)
