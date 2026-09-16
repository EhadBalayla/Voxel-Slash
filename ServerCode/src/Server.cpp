#include "Server.h"

Server* GServer = nullptr;

Server::Server(ServerProperties& properties) : 
m_NetworkManager(properties.IP, properties.PORT) 
{
    GServer = this;
}