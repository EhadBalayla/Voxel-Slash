#define MINIAUDIO_IMPLEMENTATION
#include "AudioManager.h"
#include <stdexcept>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    float* pcmOutput = reinterpret_cast<float*>(pOutput);
    for (uint32_t i = 0; i < frameCount * 2; i++) pcmOutput[i] = 0.0f;

    AudioManager* audioManager = static_cast<AudioManager*>(pDevice->pUserData);
}

void AudioManager::Init() {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 48000;
    config.dataCallback = data_callback;
    config.pUserData = this;

    if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) {
        throw std::runtime_error("failed to initialize miniaudio device");
    }

    ma_device_start(&device);
}
void AudioManager::Terminate() {
    ma_device_stop(&device);
    ma_device_uninit(&device);
}