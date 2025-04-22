add_executable(LagrangeCodecTests LagrangeCodecTests.cpp)

target_link_libraries(LagrangeCodecTests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(NAME LagrangeCodecTests COMMAND LagrangeCodecTests)