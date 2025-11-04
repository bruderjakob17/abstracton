add_executable(tests
    inductive_sets.cpp
    preach.cpp
    mata_extensions.cpp
    utils/utils.cpp
)

target_link_libraries(tests PRIVATE abstracton Catch2::Catch2WithMain)

catch_discover_tests(tests)
