#include "TickManager.h"

#include <iostream>
#include <chrono>

TickManager::TickManager() {
    tickThread = std::thread(&TickManager::TickLoop, this);
}
TickManager::~TickManager() {
    ThreadLooping = false;
    tickThread.join();
}

void TickManager::TickLoop() {
    while(ThreadLooping) {
        auto tickStart = std::chrono::steady_clock::now();

        std::cout << "Ticking Right Now, ";

        auto tickEnd = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = tickEnd - tickStart;
        float timeToWait = tickDuration - elapsed.count();

        if(timeToWait > 0.0f) {
            std::this_thread::sleep_for(std::chrono::duration<float>(timeToWait));
        }
    }
}