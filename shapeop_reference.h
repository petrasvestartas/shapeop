#pragma once
/*
 * This is a reference file for the ShapeOp library to make class navigation easier.
 * It contains the class definitions with their public methods to aid IDE navigation.
 */

namespace ShapeOp {

// Basic types
typedef double Scalar;
typedef Eigen::Matrix<Scalar, 3, 1> Vector3;
typedef Eigen::Matrix<Scalar, 3, -1> Matrix3X;

///////////////////////////////////////////////////////////////////////////////
// Constraint.h
///////////////////////////////////////////////////////////////////////////////

// Base class for all constraints
class Constraint {
public:
  // Constructor
  Constraint(const std::vector<int> &idI, Scalar weight);
  
  // Get the indices of the constrained vertices
  const std::vector<int> &getIndices() const;
  
  // Get the weight of the constraint
  Scalar getWeight() const;
  
  // Set the weight of the constraint
  void setWeight(Scalar weight);
  
  // Project the constraint
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const = 0;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const = 0;
};

// Edge strain constraint - keeps edges at specified length ranges
class EdgeStrainConstraint : public Constraint {
public:
  // Constructor
  EdgeStrainConstraint(const std::vector<int> &idI,
                       Scalar weight,
                       const Matrix3X &positions,
                       Scalar rangeMin = 1.0,
                       Scalar rangeMax = 1.0);
  
  // Project to keep edge length within specified range
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const override;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const override;
  
  // Get the minimum range factor (how much the edge can shrink)
  Scalar getRangeMin() const;
  
  // Get the maximum range factor (how much the edge can stretch)
  Scalar getRangeMax() const;
  
  // Set the range of allowed length factors
  void setRange(Scalar rangeMin, Scalar rangeMax);
  
private:
  Scalar rest_; // Original edge length
  Scalar rangeMin_; // Minimum allowed length factor
  Scalar rangeMax_; // Maximum allowed length factor
};

// Circle constraint - projects points onto a circle
class CircleConstraint : public Constraint {
public:
  // Constructor - requires at least 3 points to define a circle
  CircleConstraint(const std::vector<int> &idI,
                   Scalar weight,
                   const Matrix3X &positions);
  
  // Project to move points onto a circle
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const override;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const override;
};

// Area constraint - maintains triangle area
class AreaConstraint : public Constraint {
public:
  // Constructor - works on triangle (3 points)
  AreaConstraint(const std::vector<int> &idI,
                 Scalar weight,
                 const Matrix3X &positions,
                 Scalar rangeMin = 1.0,
                 Scalar rangeMax = 1.0);
  
  // Project to maintain triangle area within range
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const override;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const override;
  
  // Get/Set range methods similar to EdgeStrainConstraint
};

// Bending constraint - maintains bending angle between triangles
class BendingConstraint : public Constraint {
public:
  // Constructor - works on quad (4 points)
  BendingConstraint(const std::vector<int> &idI,
                    Scalar weight,
                    const Matrix3X &positions,
                    Scalar rangeMin = 0.0,
                    Scalar rangeMax = M_PI);
  
  // Project to maintain bending angle within range  
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const override;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const override;
};

// Closeness constraint - keeps points close to original positions
class ClosenessConstraint : public Constraint {
public:
  // Constructor
  ClosenessConstraint(const std::vector<int> &idI,
                      Scalar weight,
                      const Matrix3X &positions);
  
  // Project to keep points close to original positions
  virtual bool project(const Matrix3X &positions, Matrix3X &projections) const override;
  
  // Add the constraint to the global system
  virtual void addConstraint(std::vector<Eigen::Triplet<Scalar>> &triplets, int &idO) const override;
  
  // Set the goal position for the constraint
  void setPosition(const Vector3 &position);
};

///////////////////////////////////////////////////////////////////////////////
// Solver.h
///////////////////////////////////////////////////////////////////////////////

// Main solver class
class Solver {
public:
  // Constructor
  Solver();
  
  // Set the points to be optimized
  void setPoints(const Matrix3X &points);
  
  // Get the current points
  const Matrix3X &getPoints();
  
  // Add a constraint to the solver
  void addConstraint(const std::shared_ptr<Constraint> &constraint);
  
  // Add a force to the solver
  void addForces(const std::shared_ptr<Force> &force);
  
  // Initialize the solver
  bool initialize(bool dynamic = false);
  
  // Solve one iteration
  bool solve(unsigned int iterations);
};

///////////////////////////////////////////////////////////////////////////////
// Force.h
///////////////////////////////////////////////////////////////////////////////

// Base class for all forces
class Force {
public:
  // Constructor
  Force();
  
  // Add the force to all points
  virtual void addForce(Matrix3X &force) = 0;
};

// Gravity force - applies constant force in a direction
class GravityForce : public Force {
public:
  // Constructor
  GravityForce(const Vector3 &force);
  
  // Add the gravity force to all points
  virtual void addForce(Matrix3X &force) override;
  
  // Set the force vector
  void setForce(const Vector3 &force);
};

// Vertex force - applies force to specific vertices
class VertexForce : public Force {
public:
  // Constructor
  VertexForce(const Vector3 &force, const std::vector<int> &ids = std::vector<int>());
  
  // Add the force to specified vertices
  virtual void addForce(Matrix3X &force) override;
};

} // namespace ShapeOp
