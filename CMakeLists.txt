cmake_minimum_required(VERSION 3.15...3.27)
project(ShapeOpExample)

# Force the build type to Release only
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup for Ninja generator
if(CMAKE_GENERATOR MATCHES "Ninja")
    message(STATUS "Using Ninja generator")
    # Ninja-specific optimizations
    set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)
    # Enable job pools for better resource utilization
    set(CMAKE_JOB_POOL_COMPILE "compile")
    set(CMAKE_JOB_POOL_LINK "link")
    set(CMAKE_JOB_POOLS "compile=16;link=2")
endif()

# Enable parallel builds
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect number of cores for parallel compilation
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    if(CMAKE_GENERATOR MATCHES "Make")
        set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${N}")
    endif()
endif()

# Enhanced build speed settings prioritizing precompiled headers
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Reduce template instantiation depth checks for faster compile
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=256")
    
    # Speed up debug info generation
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4")
    endif()
    
    # Faster compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
    
    # Enhanced PCH optimizations 
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpch-instantiate-templates -fpch-codegen")
    endif()
endif()

# Use a more moderate optimization level to speed up compilation
# Use -O3 for release, -O1 for development
option(FAST_COMPILE "Use faster compilation settings" ON)
if(FAST_COMPILE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O1")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Add modern C++ optimization flags for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -march=native")
endif()

# Enable precompiled headers - always ON since we're not modifying ShapeOp
set(USE_PRECOMPILED_HEADERS ON)

# Include ExternalProject module
include(ExternalProject)

# Define source directories for external dependencies
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(EIGEN_SOURCE_DIR "${EXTERNAL_DIR}/eigen")
set(SHAPEOP_SOURCE_DIR "${EXTERNAL_DIR}/shapeop")
set(SHAPEOP_SRC_DIR "${SHAPEOP_SOURCE_DIR}/libShapeOp/src")
set(SHAPEOP_API_DIR "${SHAPEOP_SOURCE_DIR}/libShapeOp/api")

# Create directories if they don't exist
file(MAKE_DIRECTORY ${EXTERNAL_DIR})

# ========================================================================
# EIGEN - Download during configure phase
# ========================================================================
if(NOT EXISTS "${EIGEN_SOURCE_DIR}")
    message(STATUS "Downloading Eigen during configuration...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E echo "Downloading Eigen..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EIGEN_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/eigen.tar.gz
        WORKING_DIRECTORY ${EXTERNAL_DIR}
    )
    file(DOWNLOAD
        https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
        ${CMAKE_CURRENT_BINARY_DIR}/eigen.tar.gz
        SHOW_PROGRESS
        STATUS EIGEN_DOWNLOAD_STATUS
    )
    list(GET EIGEN_DOWNLOAD_STATUS 0 EIGEN_DOWNLOAD_STATUS_CODE)
    if(NOT EIGEN_DOWNLOAD_STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download Eigen")
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/eigen.tar.gz
        WORKING_DIRECTORY ${EXTERNAL_DIR}
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rename ${EXTERNAL_DIR}/eigen-3.4.0 ${EIGEN_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Eigen downloaded successfully"
    )
endif()

# Register Eigen for ExternalProject (mostly for dependency tracking)
ExternalProject_Add(
    eigen_download
    DOWNLOAD_COMMAND ""  # Skip download as we did it manually
    SOURCE_DIR "${EIGEN_SOURCE_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

# ========================================================================
# SHAPEOP - Download during configure phase
# ========================================================================
if(NOT EXISTS "${SHAPEOP_SOURCE_DIR}")
    message(STATUS "Downloading ShapeOp during configuration...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E echo "Cloning ShapeOp repository..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHAPEOP_SOURCE_DIR}
        COMMAND git clone https://github.com/EPFL-LGG/ShapeOp.git ${SHAPEOP_SOURCE_DIR}
        RESULT_VARIABLE SHAPEOP_CLONE_RESULT
    )
    if(NOT SHAPEOP_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone ShapeOp repository")
    endif()
    message(STATUS "ShapeOp repository cloned successfully")
endif()

# Register ShapeOp for ExternalProject (mostly for dependency tracking)
ExternalProject_Add(
    shapeop_download
    DOWNLOAD_COMMAND ""  # Skip download as we did it manually
    SOURCE_DIR "${SHAPEOP_SOURCE_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

# Create a custom target for dependencies
add_custom_target(external_downloads ALL
    DEPENDS eigen_download shapeop_download
)

# Set include directories - focusing only on the needed ShapeOp directories
set(EIGEN_INCLUDE_DIR "${EIGEN_SOURCE_DIR}")
set(SHAPEOP_INCLUDE_DIR
    "${SHAPEOP_SRC_DIR}"
    "${SHAPEOP_API_DIR}"
)

# Print include directories for verification
message(STATUS "====================================================")
message(STATUS "EIGEN INCLUDE DIRECTORY: ${EIGEN_INCLUDE_DIR}")
message(STATUS "SHAPEOP SRC DIRECTORY: ${SHAPEOP_SRC_DIR}")
message(STATUS "SHAPEOP API DIRECTORY: ${SHAPEOP_API_DIR}")
message(STATUS "====================================================")

# Add an example source file
set(EXAMPLE_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cpp_example.cpp)

# Disable Unity builds to prioritize precompiled headers
set(CMAKE_UNITY_BUILD OFF)

# Add the example executable
add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCE_FILE})
add_dependencies(${PROJECT_NAME} external_downloads)

# Include library headers
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${EIGEN_INCLUDE_DIR}
    ${SHAPEOP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # For pch.h
)

# Set up precompiled headers with enhanced settings
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

# Enhanced PCH settings for GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Add PCH-specific optimization flags
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -fno-semantic-interposition
        -Winvalid-pch              # Warn if PCH is invalid
    )
endif()

# Define SHAPEOP_HEADER_ONLY to use ShapeOp as header-only
target_compile_definitions(${PROJECT_NAME} PRIVATE SHAPEOP_HEADER_ONLY)

# Link pthread library (still needed for header-only usage)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# Add info about the build options
message(STATUS "============= Build Configuration =============")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Parallel compilation: ${N} cores")
message(STATUS "Fast compile mode: ${FAST_COMPILE}")
message(STATUS "Using precompiled headers: ENABLED (PRIORITY)")
message(STATUS "Unity build: DISABLED")
message(STATUS "ShapeOp mode: HEADER_ONLY")
message(STATUS "=============================================")

# Ninja build instructions
message(STATUS "To build with Ninja:")
message(STATUS "  1. Create a build directory: mkdir -p build")
message(STATUS "  2. Change to build directory: cd build")
message(STATUS "  3. Configure with Ninja: cmake -G Ninja ..")
message(STATUS "  4. Build the project: ninja")
