#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Rendering/FullscreenQuad.h"
#include "Rendering/ChunkRenderer.h"
#include "Rendering/Shader.h"
#include "Texture.h"
#include "AudioManager.h"
#include "ClientNetworkManager.h"
#include "ModInstance.h"
#include "../Entities/Player.h"
#include "../World/World.h"

#include "../World/Block.h"

#include "../core/Frustum.h"

#include "../ClientSideStuff/MPWorld.h"


enum class GameState {
    MainMenu,
    InGame,
    Multiplayer
};

class App {
public:
    //core managers
    Window m_Window;
    Renderer m_Renderer;
    FullscreenQuad m_FullscreenQuad;
    ChunkRenderer m_ChunkRenderer;
    AudioManager m_AudioManager;
    ClientNetworkManager m_ClientNetworkManager;
    ModInstance* m_TempMod;
    Player* m_Player;
    World* m_World;

    MPWorld* m_MPWorld = nullptr; //for the whole multiplayer thingy

    //shaders
    Shader m_OpaqueShader;
    Shader m_BorderShader;
    Shader m_BoxOutlineShader;
    Shader m_SkeletalMeshShader;
    Shader m_UIShader;
    Shader m_TextShader;
    Shader m_LightingPass;

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
    bool DoPhysics = true;
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