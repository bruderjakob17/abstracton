add_library(abstracton STATIC
    abstracton.cpp
)

target_compile_features(abstracton PUBLIC cxx_std_20)

set_target_properties(abstracton PROPERTIES
    OUTPUT_NAME abstracton
)

target_include_directories(abstracton PUBLIC "${PROJECT_SOURCE_DIR}/include/")

target_link_libraries(abstracton PUBLIC mata)
