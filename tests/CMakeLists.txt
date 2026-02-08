add_executable(LagrangeCodecTests LagrangeCodecTests.cpp)

target_include_directories(LagrangeCodecTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include/public
)

target_link_libraries(LagrangeCodecTests PRIVATE
    LagrangeCodec
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main
)

set_target_properties(LagrangeCodecTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_compile_definitions(LagrangeCodecTests PRIVATE
    LAGRANGECODEC_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data"
    $<$<BOOL:${LAGRANGECODEC_BUILD_SHARED}>:LAGRANGECODEC_SHARED>
)

if (WIN32 AND LAGRANGECODEC_BUILD_SHARED)
    add_custom_command(TARGET LagrangeCodecTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:LagrangeCodec>
            $<TARGET_FILE_DIR:LagrangeCodecTests>
    )
endif()

add_test(NAME LagrangeCodecTests COMMAND LagrangeCodecTests)
