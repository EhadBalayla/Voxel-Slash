#pragma once
#include "Window.h"
#include "Rendering/FullscreenQuad.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "AudioManager.h"
#include "AssetManager.h"
#include "../Entities/Player.h"
#include "DebugUI.h"
#include "../World/World.h"

#include "../World/Block.h"

#include "../core/Frustum.h"

enum class GameState {
    MainMenu,
    InGame
};

class App {
public:
    //core managers
    Window m_Window;
    FullscreenQuad m_FullscreenQuad;
    Renderer m_Renderer;
    AudioManager m_AudioManager;
    AssetManager m_AssetManager;
    Player* m_Player;
    World* m_World;
    DebugUI m_DebugUI;

    //shaders
    Shader m_OpaqueShader;
    Shader m_BorderShader;
    Shader m_BoxOutlineShader;
    Shader m_SkeletalMeshShader;

    //textures
    Texture m_TerrainAtlas;

    //registeries
    std::unordered_map<BlockType, BlockData> BlockRegistery;

    //misc
    glm::mat4 proj;
    int Width = 1600, Height = 900;
    float FOV = 90.0f;
    Frustum m_Frustum;
    bool showChunkBorders = false;
    bool LoadChunks = true;
    float deltaTime = 0.0f;


    App();
    void Init();
    void Loop();
    void Terminate();

    int RenderDistance = 8;
    int MaxLODLevel = 1; /*from 1 - 6*/

    GameState state = GameState::MainMenu;
    int waitingFrames = 0;
private:
    void RegisterAllBlocks();

    void LoadAllNonVoxelAssets();


    bool firstClick = false;
    float lastTime = 0.0f;
    void processInput();
};

extern App* GApp;