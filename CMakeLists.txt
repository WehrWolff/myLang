cmake_minimum_required(VERSION 3.12)
project(babel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    src/shell.cpp
)

find_package(Boost 1.83.0 REQUIRED COMPONENTS algorithm)

add_executable(babel ${SOURCE_FILES})
target_include_directories(babel PRIVATE src)
target_link_libraries(babel PRIVATE ${Boost_LIBRARIES})


# ----- Testing Configuration -----

set(TEST_FILES
    tests/test_shell.cpp
)

include_directories(src test)
find_package(GTest REQUIRED)

add_executable(babel_tests ${TEST_FILES})
target_include_directories(babel_tests PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(babel_tests PRIVATE GTest::GTest GTest::Main)
target_link_libraries(babel_tests PRIVATE ${Boost_LIBRARIES})

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# Enable testing (GTest::Main will handle test discovery and execution)
enable_testing()
add_test(NAME BabelTests COMMAND babel_tests)

#add_custom_target(coverage
#    COMMAND ${CMAKE_COMMAND} -E remove coverage.info
#    COMMAND lcov --capture --directory . --output-file coverage.info
#    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
#    COMMAND lcov --list coverage.info
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    DEPENDS babel_tests
#    COMMENT "Generating coverage information..."
#)



# Add the following lines to enable code coverage
#if (CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
#    target_compile_options(babel PRIVATE --coverage)
#    target_link_libraries(babel PRIVATE --coverage)
#endif()

# Add a custom target for generating coverage reports
#if (CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    add_custom_target(coverage
#        COMMAND lcov --directory . --capture --output-file coverage.info
#        COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
#        COMMAND lcov --list coverage.info
#        COMMAND genhtml coverage.info --output-directory coverage_report
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#        COMMENT "Generating code coverage report"
#    )
#endif()


if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Enable code coverage for GCC or Clang
        target_compile_options(babel PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(babel PRIVATE --coverage)
        
        # Add a custom target 'coverage' for generating code coverage reports
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND lcov --directory . --capture --output-file coverage.info
            COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND lcov --list coverage.info
        )
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # Enable code coverage for MSVC
        target_compile_options(babel PRIVATE /MDd /Zi /Z7)
        target_link_options(babel PRIVATE /DEBUG /OPT:REF /OPT:ICF /INCREMENTAL:NO)
        
        # Add a custom target 'coverage' for generating code coverage reports using codecov
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND codecov -f coverage.xml
        )
    endif()
endif()
