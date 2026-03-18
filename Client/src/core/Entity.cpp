#include "Entity.h"
#include "../core managers/app.h"
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "AssetFormats/TransformAsset.h"

void RenderNode(PrefabNode* node, glm::mat4 parentTransform) {
	glm::mat4 pos = glm::translate(glm::mat4(1.0f), node->pos);

	float yaw   = glm::radians(node->rot.y);
    float pitch = glm::radians(node->rot.x);
    float roll  = glm::radians(node->rot.z);
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rot = rotY * rotX * rotZ;

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), node->scale);

	glm::mat4 overall = pos * rot * scale;
	overall = parentTransform * overall;

	if(node->m_Asset) {
        node->m_Asset->Render(GApp->m_Renderer.GetFrameCommandBuffer(), GApp->m_Renderer.GetChunksPipelineLayout(), overall);
    }

	for(auto c : node->m_Children) {
		RenderNode(c, overall);
	}
}

void Entity::RenderPrefab() {
	glm::mat4 start = glm::mat4(1.0f);
	start = glm::translate(start, Position + glm::vec3(0.0f, aabb.max.y / 2.0f, 0.0f));
	start = glm::rotate(start, glm::radians(Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	RenderNode(&prefab.m_RootNode, start);
}
void Entity::RenderCollision() {

}

void Entity::MoveAndCollide(float DeltaTime) {
    float dt = DeltaTime;
    int entityHeight = 2;

    velocity.y -= gravity * dt;
	if (velocity.y < -78.4)
		velocity.y = -78.4;

	//clamp horizontal movement
	glm::vec3 temporalClampedVelocity = glm::normalize(velocity) * maxMovementSpeed; //normalized velocity only on the x and z axes not on the y
	if (velocity.x > 0.0) velocity.x = std::min(velocity.x, temporalClampedVelocity.x);
	else if (velocity.x < 0.0) velocity.x = std::max(velocity.x, temporalClampedVelocity.x);

	if (velocity.z > 0.0) velocity.z = std::min(velocity.z, temporalClampedVelocity.z);
	else if (velocity.z < 0.0) velocity.z = std::max(velocity.z, temporalClampedVelocity.z);


	//apply friction
	glm::vec3 temporalFrictionForce = glm::normalize(velocity) * friction;
	if (velocity.x != 0.0)
		if (std::abs(velocity.x) - std::abs(temporalFrictionForce.x) >= 0.0)
			velocity.x -= temporalFrictionForce.x;
		else if (std::abs(velocity.x) - std::abs(temporalFrictionForce.x) < 0.0)
			velocity.x -= temporalFrictionForce.x + (std::abs(velocity.x) - std::abs(temporalFrictionForce.x)) * -1.0;

	if (velocity.z != 0.0)
		if (std::abs(velocity.z) - std::abs(temporalFrictionForce.z) >= 0.0)
			velocity.z -= temporalFrictionForce.z;
		else if (std::abs(velocity.z) - std::abs(temporalFrictionForce.z) < 0.0)
			velocity.z -= temporalFrictionForce.z + (std::abs(velocity.z) - std::abs(temporalFrictionForce.z)) * -1.0;

	



	double dx = velocity.x * dt;
	double dy = velocity.y * dt;
	double dz = velocity.z * dt;

	glm::dvec3 newPos = Position; 
	glm::i64vec3 fPos(std::floor(Position.x), std::floor(Position.y), std::floor(Position.z)); //fPos stands for floored pos
	glm::i64vec3 pPos, nPos;
	BlockType pBlock, nBlock;



	pPos = fPos + glm::i64vec3(0, entityHeight, 0);
	nPos = fPos + glm::i64vec3(0, -1, 0);
	pBlock = GApp->m_World->GetChunkManager().GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GApp->m_World->GetChunkManager().GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.y += dy;
	if (dy > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), aabb.MovedTo(newPos))) {
		newPos.y = blockHitbox.MovedTo(pPos).min.y - entityHeight;
		velocity.y = 0.0;
	}
	else if (dy < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), aabb.MovedTo(newPos))) {
		newPos.y = blockHitbox.MovedTo(nPos).max.y;
		velocity.y = 0.0;
		IsOnGround = true;
	}
	else IsOnGround = false;



	pPos = fPos + glm::i64vec3(0, 0, 1);
	nPos = fPos + glm::i64vec3(0, 0, -1);
	pBlock = GApp->m_World->GetChunkManager().GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GApp->m_World->GetChunkManager().GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.z += dz;
	if (dz > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), aabb.MovedTo(newPos))) {
		newPos.z = pPos.z + aabb.min.z;
		velocity.z = 0.0;
	}
	else if (dz < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), aabb.MovedTo(newPos))) {
		newPos.z = nPos.z + 1.0 + aabb.max.z;
		velocity.z = 0.0;
	}



	pPos = fPos + glm::i64vec3(1, 0, 0);
	nPos = fPos + glm::i64vec3(-1, 0, 0);
	pBlock = GApp->m_World->GetChunkManager().GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GApp->m_World->GetChunkManager().GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.x += dx;
	if (dx > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), aabb.MovedTo(newPos))) {
		newPos.x = pPos.x + aabb.min.x;
		velocity.x = 0.0;
	}
	else if (dx < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), aabb.MovedTo(newPos))) {
		newPos.x = nPos.x + 1.0 + aabb.max.x;
		velocity.x = 0.0;
	}
	
	Position = newPos;
}

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