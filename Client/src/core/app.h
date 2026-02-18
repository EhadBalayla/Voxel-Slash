#pragma once
#include "Window.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Camera.h"
#include "DebugUI.h"
#include "../World/World.h"

#include "../World/Block.h"

#include "Frustum.h"

enum class GameState {
    MainMenu,
    InGame
};

class App {
public:
    //core managers
    Window m_Window;
    Renderer m_Renderer;
    Camera m_Camera;
    World* m_World;
    DebugUI m_DebugUI;

    //shaders
    Shader m_OpaqueShader;
    Shader m_BorderShader;

    //textures
    Texture m_TerrainAtlas;

    //registeries
    std::unordered_map<BlockType, BlockData> BlockRegistery;

    //misc
    glm::mat4 proj;
    int Width = 1600, Height = 900;
    float FOV = 70.0f;
    Frustum m_Frustum;
    bool showChunkBorders = false;
    bool LoadChunks = true;
    float deltaTime = 0.0f;


    App();
    void Init();
    void Loop();
    void Terminate();

    int RenderDistance = 8;
    int MaxLODLevel = 6; /*from 1 - 6*/

    GameState state = GameState::MainMenu;
    int waitingFrames = 0;
private:
    void RegisterAllBlocks();


    bool firstClick = false;
    float lastTime = 0.0f;
    void processInput();
};

extern App* GApp;