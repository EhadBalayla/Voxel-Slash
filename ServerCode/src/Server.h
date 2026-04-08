#pragma once
#include "NetworkManager.h"
#include "TickManager.h"
#include "Chunks/ChunkManager.h"
#include "EntityManager.h"
#include "Registery.h"

class Server {
public:
    Server();

    NetworkManager m_NetworkManager;
    TickManager m_TickManager;
    ChunkManager m_ChunkManager;
    EntityManager m_EntityManager;
    Registery m_Registery;
};

extern Server* GServer;