add_library(jemgui INTERFACE)

target_include_directories(jemgui INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(jemgui INTERFACE cxx_std_23)
