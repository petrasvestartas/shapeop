#include "NormalForce.h"

namespace ShapeOp {

NormalForce::NormalForce(const std::vector<std::vector<int>> &faces, double magnitude)
    : faces_(faces), magnitude_(magnitude) {}

Vector3 NormalForce::get(const Matrix3X &positions, int id) const {
    Vector3 force = Vector3::Zero();

    // Iterate over all faces
    for (const auto &face : faces_) {
        if (std::find(face.begin(), face.end(), id) != face.end()) {
            // Calculate the face normal
            Vector3 v0 = positions.col(face[0]);
            Vector3 v1 = positions.col(face[1]);
            Vector3 v2 = positions.col(face[2]);
            Vector3 normal = (v1 - v0).cross(v2 - v0).normalized();

            // Add the force contribution from this face
            force += magnitude_ * normal / face.size(); // Distribute evenly among vertices
        }
    }

    return force;
}

} // namespace ShapeOp