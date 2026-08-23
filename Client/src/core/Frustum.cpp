#include "Frustum.h"

Frustum ExtractFrustum(const glm::mat4& projView) {
    Frustum f;

    // Left
    f.planes[0].normal.x = projView[0][3] + projView[0][0];
    f.planes[0].normal.y = projView[1][3] + projView[1][0];
    f.planes[0].normal.z = projView[2][3] + projView[2][0];
    f.planes[0].d        = projView[3][3] + projView[3][0];

    // Right
    f.planes[1].normal.x = projView[0][3] - projView[0][0];
    f.planes[1].normal.y = projView[1][3] - projView[1][0];
    f.planes[1].normal.z = projView[2][3] - projView[2][0];
    f.planes[1].d        = projView[3][3] - projView[3][0];

    // Bottom
    f.planes[2].normal.x = projView[0][3] + projView[0][1];
    f.planes[2].normal.y = projView[1][3] + projView[1][1];
    f.planes[2].normal.z = projView[2][3] + projView[2][1];
    f.planes[2].d        = projView[3][3] + projView[3][1];

    // Top
    f.planes[3].normal.x = projView[0][3] - projView[0][1];
    f.planes[3].normal.y = projView[1][3] - projView[1][1];
    f.planes[3].normal.z = projView[2][3] - projView[2][1];
    f.planes[3].d        = projView[3][3] - projView[3][1];

    // Near
    f.planes[4].normal.x = projView[0][3] + projView[0][2];
    f.planes[4].normal.y = projView[1][3] + projView[1][2];
    f.planes[4].normal.z = projView[2][3] + projView[2][2];
    f.planes[4].d        = projView[3][3] + projView[3][2];

    // Normalize planes
    for (int i = 0; i < 5; i++) {
        float length = glm::length(f.planes[i].normal);
        f.planes[i].normal /= length;
        f.planes[i].d      /= length;
    }

    return f;
}
bool ChunkInFrustum(const Frustum& f, const glm::vec3& min, const glm::vec3& max) {
    for (int i = 0; i < 5; i++) {
        const Plane& p = f.planes[i];

        // Positive vertex (the furthest in the direction of the plane normal)
        glm::vec3 positive = glm::vec3(
            p.normal.x >= 0 ? max.x : min.x,
            p.normal.y >= 0 ? max.y : min.y,
            p.normal.z >= 0 ? max.z : min.z
        );

        // If positive vertex is outside → the whole AABB is outside
        if (glm::dot(p.normal, positive) + p.d < 0) {
            return false;
        }
    }
    return true;
}