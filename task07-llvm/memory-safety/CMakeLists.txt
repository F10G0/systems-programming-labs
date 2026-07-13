cmake_minimum_required(VERSION 3.18)
project(llvm-pass-memory-safety)

# Path to the LLVM installation, e.g. /usr/lib/llvm-16.
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "Path to LLVM")

# Allow CMake to locate LLVMConfig.cmake.
list(APPEND CMAKE_PREFIX_PATH
     "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM REQUIRED CONFIG)

# Use LLVM's compile definitions, headers, and library search paths.
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Match the C++ standard and RTTI configuration used by LLVM.
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif ()

# Build the instrumentation pass as a loadable shared library.
add_library(MemorySafety SHARED
    MemorySafety.cpp
)

# LLVM symbols are resolved when the plugin is loaded by opt.
target_link_libraries(MemorySafety
    "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>"
)

# Build the runtime support code as a static library.
add_library(MemorySafetyRuntimeCPP STATIC
    MemorySafetyRuntime.cpp
)
