#pragma once
#include "ClientEntityManager.h"
#include "ClientChunkManager.h"

class MPWorld {
public:
    ClientChunkManager m_ClientChunkManager;
    ClientEntityManager m_ClientEntityManager;
private:
};