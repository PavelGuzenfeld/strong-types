cmake_minimum_required(VERSION 3.22)
project(strong-types VERSION 0.1.0 LANGUAGES CXX)

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

# Test executable
add_executable(${PROJECT_NAME}_test test/strong_types_test.cpp test/si_test.cpp test/strong_aligned_array_test.cpp)
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
add_compile_options(${PROJECT_NAME}_test PRIVATE
            -Wextra            # Enable extra warnings
            -Wpedantic         # Enable pedantic warnings
            -Werror            # Treat warnings as errors
            # -Wstack-usage=4096 # Warn for large stack usage
            -pedantic # Enable pedantic warnings
            -Wconversion # Warn for implicit conversions
            -Wsign-conversion # Warn for sign conversions
            -Wdouble-promotion # Warn for implicit double promotions
            -Wformat=2 # Warn for format errors
            -Wduplicated-cond # Warn for duplicated conditions
            -Wduplicated-branches # Warn for duplicated branches
            -Wlogical-op # Warn for logical operations
            -Wuseless-cast # Warn for useless casts
            -Wshadow # Warn for shadowed variables
            -Wuninitialized # Warn for uninitialized variables
            -Wunreachable-code # Warn for unreachable code
            -Wunused # Warn for unused variables
)

# # Export the target
# install(
#     TARGETS ${PROJECT_NAME}_test
#     EXPORT ${PROJECT_NAME}Targets
#     DESTINATION bin
# )

# Handle "Manually-specified variables were not used by the project" warning.
unset(CATKIN_INSTALL_INTO_PREFIX_ROOT)
unset(CATKIN_SYMLINK_INSTALL)
