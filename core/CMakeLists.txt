add_library(rts_core STATIC
    src/battlefield.cpp
    src/tilemap_json.cpp
)

target_include_directories(rts_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# PUBLIC: tilemap_json.hpp includes nlohmann/json.hpp, so anything linking
# rts_core needs its include path too.
target_link_libraries(rts_core PUBLIC nlohmann_json)
