add_executable(rts_tests
    main.cpp
    test_battlefield.cpp
)

target_link_libraries(rts_tests PRIVATE rts_core doctest)

add_test(NAME rts_tests COMMAND rts_tests)
