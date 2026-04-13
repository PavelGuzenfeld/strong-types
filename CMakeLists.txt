cmake_minimum_required(VERSION 3.22)
project(strong-types VERSION 1.1.4 LANGUAGES CXX)

# Handle "Manually-specified variables were not used by the project" warning.
unset(CATKIN_INSTALL_INTO_PREFIX_ROOT)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Runner library
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Install public headers
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)

# Export the target
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    INCLUDES DESTINATION include
)

# Install the export file
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# Configure package files
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Install the package configuration files
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# ---- Compiler-specific warning flags ----

set(STRONG_TYPES_COMMON_WARNINGS
    -Wextra
    -Wpedantic
    -Werror
    -pedantic
    -Wconversion
    -Wsign-conversion
    -Wdouble-promotion
    -Wformat=2
    -Wshadow
    -Wuninitialized
    -Wunreachable-code
    -Wunused
)

set(STRONG_TYPES_GCC_WARNINGS
    ${STRONG_TYPES_COMMON_WARNINGS}
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
)

set(STRONG_TYPES_CLANG_WARNINGS
    ${STRONG_TYPES_COMMON_WARNINGS}
)

set(STRONG_TYPES_MSVC_WARNINGS
    /W4
    /WX
    /permissive-
)

# ---- Reusable function to apply warnings to a target ----

function(strong_types_set_warnings target)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE ${STRONG_TYPES_GCC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE ${STRONG_TYPES_CLANG_WARNINGS})
    elseif(MSVC)
        target_compile_options(${target} PRIVATE ${STRONG_TYPES_MSVC_WARNINGS})
    endif()
endfunction()

# Tests (opt-in: pass -DBUILD_TESTING=ON to build)
option(BUILD_TESTING "Build tests" OFF)
include(CTest)

if(BUILD_TESTING)
    add_executable(${PROJECT_NAME}_test
        test/strong_types_test.cpp
        test/si_test.cpp
        test/si_literals_test.cpp
        test/strong_aligned_array_test.cpp
        test/si_scaled_test.cpp
        test/si_chrono_test.cpp
        test/edge_cases_test.cpp
        test/quantity_point_test.cpp
        test/safe_math_test.cpp
    )
    target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
    strong_types_set_warnings(${PROJECT_NAME}_test)

    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

    # Optional fmt test (only built when fmt is available)
    find_package(fmt QUIET)
    if(fmt_FOUND)
        include(FetchContent)
        FetchContent_Declare(doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.4.11
        )
        FetchContent_MakeAvailable(doctest)

        add_executable(${PROJECT_NAME}_fmt_test test/fmt_test.cpp)
        target_link_libraries(${PROJECT_NAME}_fmt_test PRIVATE ${PROJECT_NAME} fmt::fmt doctest::doctest)
        strong_types_set_warnings(${PROJECT_NAME}_fmt_test)

        add_test(NAME ${PROJECT_NAME}_fmt_test COMMAND ${PROJECT_NAME}_fmt_test)
    endif()
endif()

# Fuzz testing (opt-in: pass -DBUILD_FUZZING=ON, requires Clang with libFuzzer)
option(BUILD_FUZZING "Build fuzz tests (requires Clang with libFuzzer)" OFF)
option(FUZZ_USE_LIBCXX "Use libc++ for fuzz compilation (link stays libstdc++ for fuzzer runtime)" OFF)

if(BUILD_FUZZING)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Fuzz testing requires Clang (libFuzzer)")
    endif()

    # Separate compile/link flags: libFuzzer runtime is pre-built against libstdc++,
    # so -stdlib=libc++ must only appear in compile flags (for <expected> support).
    set(FUZZ_COMPILE_FLAGS -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    set(FUZZ_LINK_FLAGS -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)

    if(FUZZ_USE_LIBCXX)
        list(APPEND FUZZ_COMPILE_FLAGS -stdlib=libc++)
    endif()

    add_executable(fuzz_safe_math test/fuzz_safe_math.cpp)
    target_link_libraries(fuzz_safe_math PRIVATE ${PROJECT_NAME})
    target_compile_options(fuzz_safe_math PRIVATE ${FUZZ_COMPILE_FLAGS})
    target_link_options(fuzz_safe_math PRIVATE ${FUZZ_LINK_FLAGS})
    strong_types_set_warnings(fuzz_safe_math)

    add_executable(fuzz_quantity_point test/fuzz_quantity_point.cpp)
    target_link_libraries(fuzz_quantity_point PRIVATE ${PROJECT_NAME})
    target_compile_options(fuzz_quantity_point PRIVATE ${FUZZ_COMPILE_FLAGS})
    target_link_options(fuzz_quantity_point PRIVATE ${FUZZ_LINK_FLAGS})
    strong_types_set_warnings(fuzz_quantity_point)
endif()

# Handle "Manually-specified variables were not used by the project" warning.
unset(CATKIN_INSTALL_INTO_PREFIX_ROOT)
unset(CATKIN_SYMLINK_INSTALL)
