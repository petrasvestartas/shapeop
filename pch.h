// pch.h - Precompiled Header for ShapeOp
#pragma once

// Standard library includes
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

// Eigen includes
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

// ShapeOp includes - use relative paths as resolved by CMake
#include "Types.h"
#include "Common.h"
#include "Constraint.h"
#include "Force.h"
#include "Solver.h"

// ShapeOp API includes
#include "API.h"
