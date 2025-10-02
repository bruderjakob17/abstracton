add_executable(tests
    inductive-sets.cpp
    preach.cpp
    mata-extensions.cpp
)

target_link_libraries(tests PRIVATE abstracton Catch2::Catch2WithMain)

catch_discover_tests(tests)
