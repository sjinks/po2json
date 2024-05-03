cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(po2json VERSION 1.0.0 LANGUAGES CXX C)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(ENABLE_TESTING "Build tests" ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;Coverage")
if(CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    set(CMAKE_DEFAULT_BUILD_TYPE "Release")
endif()

string(REGEX MATCH "Clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_CXX_COMPILER_ID}")
string(REGEX MATCH "GNU" CMAKE_COMPILER_IS_GNU "${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_COMPILER_IS_GNU)
    set(CMAKE_CXX_FLAGS_DEBUG    "-Og -g3")
    set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage -fprofile-abs-path")
elseif(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS_DEBUG    "-O0 -g3")
    set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
endif()

add_link_options($<$<AND:$<CONFIG:Coverage>,$<CXX_COMPILER_ID:GNU>>:--coverage>)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage" AND CMAKE_COMPILER_IS_GNU)
    set(CMAKE_SHARED_LINKER_FLAGS "--coverage")
endif()

add_subdirectory(src)
if(ENABLE_TESTING)
    include(CTest)
    add_subdirectory(test)

    find_program(GCOVR gcovr)
    if(GCOVR)
        if(CMAKE_COMPILER_IS_CLANG)
            set(GCOV_TOOL llvm-cov gcov)
        else()
            set(GCOV_TOOL gcov)
        endif()

        if(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
            add_custom_target(
                coverage
                COMMAND ctest -C Coverage -T test --output-on-failure
                COMMAND rm -rf "${PROJECT_BINARY_DIR}/coverage"
                COMMAND mkdir -p "${PROJECT_BINARY_DIR}/coverage"
                COMMAND gcovr -f "${PROJECT_SOURCE_DIR}" --html-details -o "${PROJECT_BINARY_DIR}/coverage/index.html" --exclude-noncode-lines --exclude-throw-branches --exclude-unreachable-branches --decisions --gcov-executable="${GCOV_TOOL}"
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            )
        endif()
    endif()
endif()
