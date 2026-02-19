#pragma once
#include <miniaudio.h>

class AudioManager {
public:
    void Init();
    void Terminate();
private:
    ma_device device;
};