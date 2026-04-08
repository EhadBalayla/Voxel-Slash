#include "Server.h"

Server* GServer = nullptr;

Server::Server() {
    GServer = this;
}