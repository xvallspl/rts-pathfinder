add_library(rts_core STATIC
    src/battlefield.cpp
)

target_include_directories(rts_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
