#pragma once
#include "NetworkManager.h"
#include "TickManager.h"
#include "Chunks/ChunkManager.h"
#include "EntityManager.h"
#include "Registery.h"

struct ServerProperties {
    char* IP;
    char* PORT;
};

class Server {
public:
    Server(ServerProperties& properties);

    //bool Create();
    //bool Terminate();

    NetworkManager m_NetworkManager;
    EntityManager m_EntityManager;
    ChunkManager m_ChunkManager;
    Registery m_Registery;
    TickManager m_TickManager;
};

extern Server* GServer;