#pragma once
#include <thread>

constexpr float tickDuration = 1.0f / 20.0f; //20 ticks per second

class TickManager {
public:
    TickManager();
    ~TickManager();
private:
    std::thread tickThread;

    bool ThreadLooping = true;
    void TickLoop();
};