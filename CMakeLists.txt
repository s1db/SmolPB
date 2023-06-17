cmake_minimum_required(VERSION 3.12)
project(SmolPB)

set(CMAKE_CXX_STANDARD 20)

# Enable compiler warnings
add_compile_options(-Wall -Wextra -pedantic)

# Enable strict compile flags (treat warnings as errors)
add_compile_options(-Werror)

# Add the source files to the project
set(SOURCES
    src/main.cpp
    src/constraint.cpp
    src/model_evaluator.cpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Download and include Catch2 library
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.2
)
FetchContent_MakeAvailable(Catch2)

# Include the Catch2 directories
include_directories(${catch2_SOURCE_DIR}/single_include)

# Add the tests
add_executable(tests
    tests/test_main.cpp
    tests/test_constraint.cpp
    tests/test_model_evaluator.cpp
    src/constraint.cpp
    src/model_evaluator.cpp
)

# Link the Catch2 library
target_link_libraries(tests PRIVATE Catch2::Catch2)

# Include the project's include directories for tests
target_include_directories(tests PRIVATE
    include
    src
)

# Register the tests with CTest
include(CTest)
enable_testing()
add_test(NAME tests COMMAND tests)
