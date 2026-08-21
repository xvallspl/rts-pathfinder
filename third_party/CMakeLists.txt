# Vendored, header-only third-party dependencies. See README's Third-Party
# Libraries section for what each is used for and why it was chosen.
#
# SYSTEM: these headers are not ours to fix, so their warnings shouldn't
# drown out ours (MSVC /W4 flags doctest for specializing std::tuple).

add_library(doctest INTERFACE)
target_include_directories(doctest SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
