cmake_minimum_required(VERSION 3.5)
project(cpp_example)

# Force CMake to use a specific build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fast compilation option (ON = faster compilation, OFF = faster execution)
option(FAST_COMPILE "Optimize for faster compilation" ON)
if(FAST_COMPILE)
  # Favor faster compilation over runtime performance
  add_compile_options(-O0)
else()
  # Favor runtime performance over compilation speed
  add_compile_options(-O3)
endif()

# Ninja is faster for incremental builds
if (CMAKE_GENERATOR MATCHES "Ninja")
  message(STATUS "Using Ninja generator")
endif()

# External dependencies setup (Eigen and ShapeOp)
set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen)
set(SHAPEOP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/shapeop/libShapeOp/src)
set(SHAPEOP_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/shapeop/libShapeOp/api)
set(SHAPEOP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/shapeop/libShapeOp)

# Download dependencies during configuration phase
include(ExternalProject)

# Download Eigen (during configure phase, not build phase)
if(NOT EXISTS ${EIGEN_INCLUDE_DIR})
  message(STATUS "Downloading Eigen...")
  file(DOWNLOAD
    https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
    ${CMAKE_CURRENT_SOURCE_DIR}/eigen.zip
    SHOW_PROGRESS
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_SOURCE_DIR}/eigen.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E rename 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen-3.4.0 
    ${EIGEN_INCLUDE_DIR}
  )
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/eigen.zip)
endif()

# ShapeOp download in the configuration phase
if(NOT EXISTS ${SHAPEOP_INCLUDE_DIR})
  message(STATUS "Downloading ShapeOp...")
  file(DOWNLOAD
    https://github.com/EPFL-LGG/ShapeOp/archive/refs/heads/master.zip
    ${CMAKE_CURRENT_SOURCE_DIR}/shapeop.zip
    SHOW_PROGRESS
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_SOURCE_DIR}/shapeop.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E rename 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/ShapeOp-master 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/shapeop
  )
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/shapeop.zip)
endif()

# ExternalProject for tracking dependencies, but download already done above
ExternalProject_Add(external_downloads
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/external
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

# Build ShapeOp as a library (not header-only) for faster incremental builds
add_library(shapeop STATIC
    ${SHAPEOP_SRC_DIR}/Constraint.cpp
    ${SHAPEOP_SRC_DIR}/Force.cpp
    ${SHAPEOP_SRC_DIR}/LSSolver.cpp
    ${SHAPEOP_SRC_DIR}/Solver.cpp
)

target_include_directories(shapeop PRIVATE
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

# Compile with optimization for the library
target_compile_options(shapeop PRIVATE -O3)

# Set target sources for our example
set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cpp_example.cpp)

# Add the main executable
add_executable(cpp_example ${EXAMPLE_SOURCE})
add_dependencies(cpp_example external_downloads)

# Find required packages
find_package(Threads REQUIRED)

# Link against ShapeOp library
target_link_libraries(cpp_example PRIVATE shapeop Threads::Threads)

# The main executable needs to include all the ShapeOp headers
target_include_directories(cpp_example PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

# Set up precompiled headers
target_precompile_headers(cpp_example PRIVATE pch.h)

# Build configuration info
message(STATUS "============= Build Configuration =============")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Optimization: ${FAST_COMPILE} (O0 if ON, O3 if OFF)")
message(STATUS "Using precompiled headers: ENABLED")
message(STATUS "ShapeOp mode: LIBRARY")
message(STATUS "=============================================")
