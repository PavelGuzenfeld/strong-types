# Compiler warning flags for C++ projects.
# Usage: include(cmake_warnings.cmake) then target_link_libraries(mylib PRIVATE warnings)
#
# Creates an INTERFACE library target "warnings" with recommended flags.
# Supports GCC and Clang.

add_library(warnings INTERFACE)

# GCC and Clang common flags
target_compile_options(warnings INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Werror

  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wconversion
  -Wsign-conversion
  -Wnull-dereference
  -Wdouble-promotion
  -Wformat=2
  -Wimplicit-fallthrough
)

# GCC-specific
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(warnings INTERFACE
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
    -Wsuggest-override
  )
endif()

# Clang-specific
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(warnings INTERFACE
    -Wno-unknown-warning-option
  )
endif()
