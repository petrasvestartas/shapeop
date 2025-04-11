cmake_minimum_required(VERSION 3.10)
project(example)

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

# Create external directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external)

# External dependencies setup (Eigen and ShapeOp)
set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen)
set(SHAPEOP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/shapeop)
set(SHAPEOP_SRC_DIR ${SHAPEOP_SOURCE_DIR}/libShapeOp/src)
set(SHAPEOP_API_DIR ${SHAPEOP_SOURCE_DIR}/libShapeOp/api)
set(SHAPEOP_INCLUDE_DIR ${SHAPEOP_SOURCE_DIR}/libShapeOp)

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
  
  # Extract Eigen
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_SOURCE_DIR}/eigen.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
    RESULT_VARIABLE EIGEN_EXTRACT_RESULT
  )
  
  if(NOT EIGEN_EXTRACT_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to extract Eigen")
  endif()
  
  # Rename extracted directory
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen-3.4.0)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E rename 
        ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen-3.4.0 
        ${EIGEN_INCLUDE_DIR}
      RESULT_VARIABLE EIGEN_RENAME_RESULT
    )
    
    if(NOT EIGEN_RENAME_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to rename Eigen directory")
    endif()
  else()
    message(FATAL_ERROR "Eigen extraction successful but directory eigen-3.4.0 not found")
  endif()
  
  # Cleanup
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/eigen.zip)
endif()

# ShapeOp download in the configuration phase
if(NOT EXISTS ${SHAPEOP_SOURCE_DIR})
  message(STATUS "Downloading ShapeOp...")
  file(DOWNLOAD
    https://github.com/EPFL-LGG/ShapeOp/archive/refs/heads/master.zip
    ${CMAKE_CURRENT_SOURCE_DIR}/shapeop.zip
    SHOW_PROGRESS
  )
  
  # Extract ShapeOp
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_SOURCE_DIR}/shapeop.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
    RESULT_VARIABLE SHAPEOP_EXTRACT_RESULT
  )
  
  if(NOT SHAPEOP_EXTRACT_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to extract ShapeOp")
  endif()
  
  # Rename extracted directory
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/ShapeOp-master)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E rename 
        ${CMAKE_CURRENT_SOURCE_DIR}/external/ShapeOp-master 
        ${SHAPEOP_SOURCE_DIR}
      RESULT_VARIABLE SHAPEOP_RENAME_RESULT
    )
    
    if(NOT SHAPEOP_RENAME_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to rename ShapeOp directory")
    endif()
  else()
    message(FATAL_ERROR "ShapeOp extraction successful but directory ShapeOp-master not found")
  endif()
  
  # Cleanup
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

# Verify that required files exist before trying to build
if(NOT EXISTS ${SHAPEOP_SRC_DIR}/Constraint.cpp)
  message(FATAL_ERROR "ShapeOp source files missing - download problem. Expected: ${SHAPEOP_SRC_DIR}/Constraint.cpp")
endif()

# Build ShapeOp as a library (not header-only) for faster incremental builds
add_library(shapeop STATIC
    ${SHAPEOP_SRC_DIR}/Constraint.cpp
    ${SHAPEOP_SRC_DIR}/Force.cpp
    ${SHAPEOP_SRC_DIR}/LSSolver.cpp
    ${SHAPEOP_SRC_DIR}/Solver.cpp
    src/NormalForce.cpp
)

target_include_directories(shapeop PRIVATE
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

# Compile with optimization for the library
target_compile_options(shapeop PRIVATE -O3)

# Set the default example to build
set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/wind_cloth.cpp)

# Enable compile_commands.json generation for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the main example executable
add_executable(example ${EXAMPLE_SOURCE})
target_link_libraries(example
  shapeop
)
add_dependencies(example external_downloads)

# Add additional example executables
add_executable(wind_cloth_bin wind_cloth.cpp)
add_executable(cable_net_bin cable_net.cpp)
add_executable(balloon_bin balloon.cpp)

target_link_libraries(wind_cloth_bin shapeop)
target_link_libraries(cable_net_bin shapeop)
target_link_libraries(balloon_bin shapeop)

add_dependencies(wind_cloth_bin external_downloads)
add_dependencies(cable_net_bin external_downloads)
add_dependencies(balloon_bin external_downloads)

# The main executable needs to include all the ShapeOp headers
target_include_directories(example PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

# The additional examples need the same include directories
target_include_directories(wind_cloth_bin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

target_include_directories(cable_net_bin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

target_include_directories(balloon_bin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${EIGEN_INCLUDE_DIR}
  ${SHAPEOP_INCLUDE_DIR}
  ${SHAPEOP_SRC_DIR}
  ${SHAPEOP_API_DIR}
)

# Set up precompiled headers
target_precompile_headers(example PRIVATE pch.h)

# Build configuration info
message(STATUS "============= Build Configuration =============")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Optimization: ${FAST_COMPILE} (O0 if ON, O3 if OFF)")
message(STATUS "Using precompiled headers: ENABLED")
message(STATUS "ShapeOp mode: LIBRARY")
message(STATUS "=============================================")
