set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(GTest REQUIRED)

add_executable(test_po2json)
target_compile_features(test_po2json PRIVATE cxx_std_20)
set_target_properties(test_po2json PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/src"
)

add_dependencies(test_po2json objlib)

set(
    TEST_SOURCES
    test_conv.cpp
    test_json.cpp
    test_processor.cpp
    test_translations.cpp
)

target_sources(
    test_po2json PRIVATE
    ${TEST_SOURCES}
    $<TARGET_OBJECTS:objlib>
)

target_link_libraries(test_po2json PUBLIC GTest::gtest GTest::gtest_main gettextpo)

if(UNIX)
    add_custom_command(TARGET test_po2json POST_BUILD COMMAND find . -type f -name \*.gcda -delete WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(GoogleTest)
gtest_add_tests(TARGET test_po2json SOURCES ${TEST_SOURCES})
