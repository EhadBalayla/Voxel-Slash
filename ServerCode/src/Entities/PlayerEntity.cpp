#include "PlayerEntity.h"

#include "../Core Stuff/Entity.h"
#include "../Server.h"
#include "../Core Stuff/Chunk.h"

#undef min
#undef max

void MoveAndCollide(Entity* self, float DeltaTime) {
    PlayerData* pData = reinterpret_cast<PlayerData*>(self->ExtraData);

    float dt = DeltaTime;
    int entityHeight = 2;

    pData->velocity.y -= pData->gravity * dt;
	if (pData->velocity.y < -78.4)
		pData->velocity.y = -78.4;

	//clamp horizontal movement
	glm::vec3 temporalClampedVelocity = glm::normalize(pData->velocity) * pData->maxMovementSpeed; //normalized velocity only on the x and z axes not on the y
	if (pData->velocity.x > 0.0) pData->velocity.x = std::min(pData->velocity.x, temporalClampedVelocity.x);
	else if (pData->velocity.x < 0.0) pData->velocity.x = std::max(pData->velocity.x, temporalClampedVelocity.x);

	if (pData->velocity.z > 0.0) pData->velocity.z = std::min(pData->velocity.z, temporalClampedVelocity.z);
	else if (pData->velocity.z < 0.0) pData->velocity.z = std::max(pData->velocity.z, temporalClampedVelocity.z);


	//apply friction
	glm::vec3 temporalFrictionForce = glm::normalize(pData->velocity) * pData->friction;
	if (pData->velocity.x != 0.0)
		if (std::abs(pData->velocity.x) - std::abs(temporalFrictionForce.x) >= 0.0)
			pData->velocity.x -= temporalFrictionForce.x;
		else if (std::abs(pData->velocity.x) - std::abs(temporalFrictionForce.x) < 0.0)
			pData->velocity.x -= temporalFrictionForce.x + (std::abs(pData->velocity.x) - std::abs(temporalFrictionForce.x)) * -1.0;

	if (pData->velocity.z != 0.0)
		if (std::abs(pData->velocity.z) - std::abs(temporalFrictionForce.z) >= 0.0)
			pData->velocity.z -= temporalFrictionForce.z;
		else if (std::abs(pData->velocity.z) - std::abs(temporalFrictionForce.z) < 0.0)
			pData->velocity.z -= temporalFrictionForce.z + (std::abs(pData->velocity.z) - std::abs(temporalFrictionForce.z)) * -1.0;

	



	double dx = pData->velocity.x * dt;
	double dy = pData->velocity.y * dt;
	double dz = pData->velocity.z * dt;

	glm::dvec3 newPos = self->Position; 
	glm::i64vec3 fPos(std::floor(self->Position.x), std::floor(self->Position.y), std::floor(self->Position.z)); //fPos stands for floored pos
	glm::i64vec3 pPos, nPos;
	BlockType pBlock, nBlock;



	pPos = fPos + glm::i64vec3(0, entityHeight, 0);
	nPos = fPos + glm::i64vec3(0, -1, 0);
	pBlock = GServer->m_ChunkManager.GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GServer->m_ChunkManager.GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.y += dy;
	if (dy > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.y = blockHitbox.MovedTo(pPos).min.y - entityHeight;
		pData->velocity.y = 0.0;
	}
	else if (dy < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.y = blockHitbox.MovedTo(nPos).max.y;
		pData->velocity.y = 0.0;
		pData->IsOnGround = true;
	}
	else pData->IsOnGround = false;



	pPos = fPos + glm::i64vec3(0, 0, 1);
	nPos = fPos + glm::i64vec3(0, 0, -1);
	pBlock = GServer->m_ChunkManager.GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GServer->m_ChunkManager.GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.z += dz;
	if (dz > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.z = pPos.z + self->Data.aabb.min.z;
		pData->velocity.z = 0.0;
	}
	else if (dz < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.z = nPos.z + 1.0 + self->Data.aabb.max.z;
		pData->velocity.z = 0.0;
	}



	pPos = fPos + glm::i64vec3(1, 0, 0);
	nPos = fPos + glm::i64vec3(-1, 0, 0);
	pBlock = GServer->m_ChunkManager.GetBlockAt(pPos.x, pPos.y, pPos.z);
	nBlock = GServer->m_ChunkManager.GetBlockAt(nPos.x, nPos.y, nPos.z);

	newPos.x += dx;
	if (dx > 0.0 && pBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(pPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.x = pPos.x + self->Data.aabb.min.x;
		pData->velocity.x = 0.0;
	}
	else if (dx < 0.0 && nBlock != BlockType::Air && Intersects(blockHitbox.MovedTo(nPos), self->Data.aabb.MovedTo(newPos))) {
		newPos.x = nPos.x + 1.0 + self->Data.aabb.max.x;
		pData->velocity.x = 0.0;
	}
	
	self->Position = newPos;
}

void PlayerTick(Entity* self) {
    PlayerData* pData = reinterpret_cast<PlayerData*>(self->ExtraData);

    //UpdateChunksAroundPlayer(self);
	if (pData->IsJump) {
        if(pData->IsOnGround) pData->velocity.y = 10.0f;
    }
    if(pData->IsForward) {
        pData->velocity += glm::vec3(0.0f, 0.0f, 1.0f) * pData->acceleration * 10.0f;
    }
    if(pData->IsBackward) {
        pData->velocity += glm::vec3(0.0f, 0.0f, -1.0f) * pData->acceleration * 10.0f;
    }
    if(pData->IsLeft) {
        pData->velocity += glm::vec3(1.0f, 0.0f, 0.0f) * pData->acceleration * 10.0f;
    }
    if(pData->IsRight) {
        pData->velocity += glm::vec3(-1.0f, 0.0f, 0.0f) * pData->acceleration * 10.0f;
    }

    MoveAndCollide(self, 1.0 / 20.0f);
}
void* PlayerDataCreation() {
    PlayerData* pData = (PlayerData*)malloc(sizeof(PlayerData)); 

    pData->ChunkCoordX = 0;
    pData->ChunkCoordY = 0;
    pData->ChunkCoordZ = 0;

    pData->IsOnGround = true;
    pData->acceleration = 0.5f;
	pData->maxMovementSpeed = 5.2f;
	pData->friction = 0.8f;
	pData->gravity = 15.0f;

    pData->IsForward = true;
    pData->IsBackward = false;
    pData->IsLeft = false;
    pData->IsRight = false;

    return pData;
}