add_executable(rts_tests
    main.cpp
    test_battlefield.cpp
    test_tilemap_json.cpp
)

target_link_libraries(rts_tests PRIVATE rts_core doctest)

target_compile_definitions(rts_tests PRIVATE RTS_SAMPLES_DIR="${CMAKE_SOURCE_DIR}/samples")

add_test(NAME rts_tests COMMAND rts_tests)
