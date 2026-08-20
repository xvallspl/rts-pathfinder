# Vendored, header-only third-party dependencies. See README's Third-Party
# Libraries section for what each is used for and why it was chosen.

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
