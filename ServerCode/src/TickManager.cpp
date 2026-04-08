#include "TickManager.h"
#include "Server.h"
#include <iostream>
#include <chrono>

TickManager::TickManager() {
    tickThread = std::thread(&TickManager::TickLoop, this);
    std::cout << "Started off the ticking thread" << std::endl;
}
TickManager::~TickManager() {
    ThreadLooping = false;
    tickThread.join();
    std::cout << "Stopped the ticking thread" << std::endl;
}

void TickManager::TickLoop() {
    while(ThreadLooping) {
        auto tickStart = std::chrono::steady_clock::now();
        
        GServer->m_EntityManager.TickEntities();

        auto tickEnd = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = tickEnd - tickStart;
        float timeToWait = tickDuration - elapsed.count();

        if(timeToWait > 0.0f) {
            std::this_thread::sleep_for(std::chrono::duration<float>(timeToWait));
        }
    }
}