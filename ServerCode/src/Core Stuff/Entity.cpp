#include "Entity.h"

glm::vec3 Entity::GetForwardVector() {
    float yawRadians = glm::radians(Rotation);
	glm::vec3 forward;
	forward.x = cos(yawRadians);
	forward.y = 0.0f;
	forward.z = sin(yawRadians);
	return forward;
}
glm::vec3 Entity::GetRightVector() {
    return glm::normalize(glm::cross(GetForwardVector(), glm::vec3(0.0f, 1.0f, 0.0f)));
}