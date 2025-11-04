add_library(mata_extensions STATIC
    mata_extensions.cpp
)
add_library(abstracton STATIC
    abstracton.cpp
)

target_compile_features(abstracton PUBLIC cxx_std_20)

set_target_properties(abstracton PROPERTIES
    OUTPUT_NAME abstracton
)

target_include_directories(mata_extensions PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_include_directories(abstracton PUBLIC "${PROJECT_SOURCE_DIR}/include/")

target_link_libraries(mata_extensions PUBLIC mata)
target_link_libraries(abstracton PUBLIC mata)
target_link_libraries(abstracton PUBLIC mata_extensions)
