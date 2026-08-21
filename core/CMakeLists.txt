add_library(rts_core STATIC
    src/battlefield.cpp
    src/pathfinder.cpp
    src/tilemap_json.cpp
)

target_include_directories(rts_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# PRIVATE: JSON is an implementation detail of the parser -- it appears in
# no public header, so consumers of rts_core don't need its include path.
target_link_libraries(rts_core PRIVATE nlohmann_json)
