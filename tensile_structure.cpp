#include "pch.h"
#include "Solver.h"
#include "Constraint.h"
#include "Force.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <random>

int main() {
    // Create a minimal surface tensile structure that spans between fixed boundary points
    
    // Parameters for the mesh
    const int rows = 30;
    const int cols = 30;
    const double spacing = 0.1;
    const double boundaryHeight = 0.5; // Height of the fixed boundary points
    
    // Random number generator for initial mesh perturbation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(-0.1, 0.1);
    
    // Total number of points in the grid mesh
    const int n_points = rows * cols;
    
    // Initialize points matrix
    ShapeOp::Matrix3X points(3, n_points);
    
    // Helper function to get the index of a grid point
    auto index = [cols](int x, int y) { return y * cols + x; };
    
    // Keep track of boundary points
    std::vector<int> boundaryIndices;
    
    // Create initial mesh as a regular grid with fixed elevated boundaries
    // and a slightly randomized center
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            const int i = index(x, y);
            
            double posX = x * spacing;
            double posY = y * spacing;
            double posZ = 0.0;
            
            // Is this a boundary point?
            bool isBoundary = (x == 0 || x == cols - 1 || y == 0 || y == rows - 1);
            
            if (isBoundary) {
                // Boundaries follow a sinusoidal wave pattern for interesting elevated fixed points
                double normalizedX = static_cast<double>(x) / (cols - 1);
                double normalizedY = static_cast<double>(y) / (rows - 1);
                
                posZ = boundaryHeight * 
                       (sin(normalizedX * 2 * M_PI) * 0.5 + 0.5) * 
                       (sin(normalizedY * 2 * M_PI) * 0.5 + 0.5);
                       
                boundaryIndices.push_back(i);
            } else {
                // Interior points have slight random perturbation for a better starting point
                posZ = distrib(gen);
            }
            
            points.col(i) = ShapeOp::Vector3(posX, posY, posZ);
        }
    }
    
    // Create the solver and set the points
    ShapeOp::Solver solver;
    solver.setPoints(points);
    
    // Fix the boundary points
    for (int idx : boundaryIndices) {
        std::vector<int> fixedPoint = {idx};
        auto constraint = std::make_shared<ShapeOp::ClosenessConstraint>(
            fixedPoint, 1e5, solver.getPoints());
        solver.addConstraint(constraint);
    }
    
    // Connect all adjacent points with edge strain constraints
    // We'll use tight constraints to simulate tensile forces
    double edgeWeight = 10.0;
    
    // Horizontal edges
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            std::vector<int> edgeIndices = {index(x, y), index(x+1, y)};
            
            // Allow stretching but resist compression (typical for tensile structures)
            auto constraint = std::make_shared<ShapeOp::EdgeStrainConstraint>(
                edgeIndices, edgeWeight, solver.getPoints(), 0.9, 1.1);
            solver.addConstraint(constraint);
        }
    }
    
    // Vertical edges
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols; ++x) {
            std::vector<int> edgeIndices = {index(x, y), index(x, y+1)};
            auto constraint = std::make_shared<ShapeOp::EdgeStrainConstraint>(
                edgeIndices, edgeWeight, solver.getPoints(), 0.9, 1.1);
            solver.addConstraint(constraint);
        }
    }
    
    // Add diagonal edges to prevent shearing
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            // Diagonal from top-left to bottom-right
            {
                std::vector<int> edgeIndices = {index(x, y), index(x+1, y+1)};
                auto constraint = std::make_shared<ShapeOp::EdgeStrainConstraint>(
                    edgeIndices, edgeWeight * 0.7, solver.getPoints(), 0.9, 1.2);
                solver.addConstraint(constraint);
            }
            
            // Diagonal from top-right to bottom-left
            {
                std::vector<int> edgeIndices = {index(x+1, y), index(x, y+1)};
                auto constraint = std::make_shared<ShapeOp::EdgeStrainConstraint>(
                    edgeIndices, edgeWeight * 0.7, solver.getPoints(), 0.9, 1.2);
                solver.addConstraint(constraint);
            }
        }
    }
    
    // Add area constraints to minimize surface area (aiming for minimal surface)
    double areaWeight = 1.0;
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            // Create triangle 1 indices (top-left, bottom-left, bottom-right)
            {
                std::vector<int> triangleIndices = {
                    index(x, y), 
                    index(x, y+1), 
                    index(x+1, y+1)
                };
                
                auto constraint = std::make_shared<ShapeOp::AreaConstraint>(
                    triangleIndices, areaWeight, solver.getPoints(), 0.0, 0.5);
                solver.addConstraint(constraint);
            }
            
            // Create triangle 2 indices (top-left, bottom-right, top-right)
            {
                std::vector<int> triangleIndices = {
                    index(x, y), 
                    index(x+1, y+1), 
                    index(x+1, y)
                };
                
                auto constraint = std::make_shared<ShapeOp::AreaConstraint>(
                    triangleIndices, areaWeight, solver.getPoints(), 0.0, 0.5);
                solver.addConstraint(constraint);
            }
        }
    }
    
    // Add a very light gravity force to help pull the structure down
    ShapeOp::Vector3 gravityDir(0.0, 0.0, -0.05);
    auto gravity = std::make_shared<ShapeOp::GravityForce>(gravityDir);
    solver.addForces(gravity);
    
    // Initialize and solve
    solver.initialize(false); // Non-dynamic solver for pure form-finding
    
    std::cout << "Optimizing tensile structure... ";
    const int num_iterations = 1000;
    for (int i = 0; i < num_iterations; i++) {
        solver.solve(1);
        
        // Print progress every 200 iterations
        if (i % 200 == 0) {
            std::cout << i << "... ";
        }
    }
    std::cout << "done." << std::endl;
    
    // Get the final optimized points
    ShapeOp::Matrix3X optimized_points = solver.getPoints();
    
    // Save the mesh to an OBJ file
    std::ofstream objFile("tensile_structure.obj");
    
    // Write vertices
    for (int i = 0; i < n_points; ++i) {
        objFile << "v " 
                << optimized_points(0, i) << " " 
                << optimized_points(1, i) << " " 
                << optimized_points(2, i) << std::endl;
    }
    
    // Write faces (using 1-indexed vertices as per OBJ format)
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            int i0 = index(x, y) + 1;      // +1 because OBJ indices start at 1
            int i1 = index(x+1, y) + 1;
            int i2 = index(x+1, y+1) + 1;
            int i3 = index(x, y+1) + 1;
            
            // Create two triangular faces per quad
            objFile << "f " << i0 << " " << i1 << " " << i2 << std::endl;
            objFile << "f " << i0 << " " << i2 << " " << i3 << std::endl;
        }
    }
    
    objFile.close();
    std::cout << "Mesh written to tensile_structure.obj" << std::endl;
    
    return 0;
}
