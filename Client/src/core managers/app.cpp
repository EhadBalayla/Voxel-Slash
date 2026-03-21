#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "app.h"
#include "../core/Utilities.h"

#include <iostream>
#include <filesystem>

#include "Canvas.h"

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void resize_callback(GLFWwindow* window, int width, int height);

App* GApp = nullptr;
App::App() {
    GApp = this;
}

void App::Init() {
    Window::InitGLFW();
    m_Window.CreateWindow("Voxel Slash", Width, Height);
    m_Window.MakeContext();
    glfwSetCursorPosCallback(m_Window.GetGLFWwindow(), mouse_callback);
    glfwSetFramebufferSizeCallback(m_Window.GetGLFWwindow(), resize_callback);

    m_Renderer.SetHandles(
        m_Window.GetContext().GetInstance(),
        m_Window.GetContext().GetPhysicalDevice(),
        m_Window.GetContext().GetDevice(),
        m_Window.GetContext().GetGraphicsQueue(),
        m_Window.GetContext().GetPresentQueue(),
        m_Window.GetContext().GetSurface(),
        m_Window.GetContext().GetGraphicsFamily(),
        m_Window.GetContext().GetPresentFamily(),
        m_Window.GetContext().GetCommandPool(),
        m_Window.GetContext().GetCommandBuffers(),
        m_Window.GetContext().GetAllocator(),
        m_Window.GetContext().MAX_FRAMES_IN_FLIGHT,
        &m_Window.GetContext().currentFrame
    );
    m_Renderer.Init();

    m_FullscreenQuad.CreateFullscreenQuad();

    m_DebugUI.Init();


    m_AudioManager.Init();

    
    //load textures
    m_TerrainAtlas.LoadTexture("assets/Textures/TerrainAtlas.png");
    
    //load the descriptor sets
    m_Renderer.StartDescriptors();

    //load shaders
    m_OpaqueShader.LoadShader("assets/Shaders/Opaque_vert.spv", "assets/Shaders/Opaque_frag.spv", PipelineType::Chunk);
    m_BorderShader.LoadShader("assets/Shaders/ChunkBorder_vert.spv", "assets/Shaders/ChunkBorder_frag.spv", PipelineType::BoxOutline);
    m_BoxOutlineShader.LoadShader("assets/Shaders/BoxOutline_vert.spv", "assets/Shaders/BoxOutline_frag.spv", PipelineType::BoxOutline);
    m_SkeletalMeshShader.LoadShader("assets/Shaders/SkeletalMeshShader_vert.spv", "assets/Shaders/SkeletalMeshShader_frag.spv", PipelineType::SkeletalMesh);
    m_UIShader.LoadShader("assets/Shaders/UIShader_vert.spv", "assets/Shaders/UIShader_frag.spv", PipelineType::UIShader);

    RegisterAllBlocks();

    m_TempMod = new ModInstance("Data");
    m_Player = new Player;
}
void App::Loop() {
    while(!m_Window.ShouldClose()) {
        m_Window.StartFrame();
        m_Window.PollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        switch(state) {
            case GameState::MainMenu: {
                m_Renderer.StartRender();
                m_UIShader.Bind();
                Canvas* titleScr = m_TempMod->GetAllCanvases()["TitleScreenHUD"];
                titleScr->Render(m_Renderer.GetFrameCommandBuffer(), m_Renderer.GetChunksPipelineLayout(), Width, Height);
                m_Renderer.EndRender();

                m_FullscreenQuad.SetTexture();

                m_Window.StartFullscreenRender();
                m_FullscreenQuad.Draw();
                m_DebugUI.RenderMenuDebugUI();
                m_Window.EndFullscreenRender();
                break;
            }
            case GameState::InGame: {
                if(waitingFrames == 0) {
                processInput();
                proj = glm::perspective(glm::radians(FOV), Width / static_cast<float>(Height), 0.1f, 50000.0f);
                m_Frustum = ExtractFrustum(proj * m_Player->GetViewMatrix());


                m_Player->UpdateChunksAroundPlayer();
                m_Player->Update(deltaTime);

                m_Renderer.SetViewProj(m_Player->GetViewMatrix(), proj);
                     
                m_Renderer.StartRender();
                m_World->RenderWorld();
                
                {
                    glm::mat4 mat = glm::mat4(1.0f);
                    mat = glm::translate(glm::mat4(1.0f), m_Player->Position + glm::vec3(0.0f, m_Player->aabb.max.y / 2.0f, 0.0f));
                    mat = glm::scale(mat, glm::vec3(0.5f, m_Player->aabb.max.y, 0.5f));

                    m_BoxOutlineShader.Bind();
                    m_Renderer.SetTrans(mat);
                    vkCmdDraw(m_Window.GetContext().GetCommandBuffers()[m_Window.GetContext().currentFrame], 24, 1, 0, 0);

                    m_SkeletalMeshShader.Bind();
                    m_Player->RenderPrefab();
                }

                if(showChunkBorders) {
                    m_Renderer.SetTrans(glm::translate(glm::mat4(1.0f), glm::vec3(m_Player->ChunkCoordX * 32, m_Player->ChunkCoordY * 32,m_Player->ChunkCoordZ * 32)));
                
                    m_Renderer.BindVoxelDescriptor();
                    m_BorderShader.Bind();
                    vkCmdDraw(m_Window.GetContext().GetCommandBuffers()[m_Window.GetContext().currentFrame], 36, 1, 0, 0);
                }
                m_Renderer.EndRender();

                m_FullscreenQuad.SetTexture();

                m_Window.StartFullscreenRender();
                m_FullscreenQuad.Draw();
                m_DebugUI.RenderDebugUI();
                m_Window.EndFullscreenRender();
                } else {
                    waitingFrames++;
                    if(waitingFrames == 4) { 
                        state = GameState::MainMenu;

                        delete GApp->m_World;
                    }
                }
                break;
            }
        }
        m_Window.NextFrame();
    }
}
void App::Terminate() {
    vkDeviceWaitIdle(m_Renderer.GetDevice());

    m_BorderShader.UnloadShader();
    m_OpaqueShader.UnloadShader();

    m_Renderer.EndDescriptors();

    m_TerrainAtlas.UnloadTexture();

    m_AudioManager.Terminate();
    
    m_DebugUI.Terminate();
    
    m_Renderer.Terminate();
    

    m_Window.DestroyContext();
    m_Window.DestroyWindow();
    Window::TerminateGLFW();
}

void App::RegisterAllBlocks() {
    BlockRegistery[BlockType::Air] = {{0, 0, 0, 0, 0, 0}};
    BlockRegistery[BlockType::Stone] = {{3, 3, 3, 3, 3, 3}};
    BlockRegistery[BlockType::Grass] = {{0, 2, 1, 1, 1, 1}};
    BlockRegistery[BlockType::Dirt] = {{2, 2, 2, 2, 2, 2}};
    BlockRegistery[BlockType::Mystery] = {{4, 4, 4, 4, 4, 4}};
}



bool firstMouse = false;
float lastX = 1280.0f/2.0f;
float lastY = 720.0f/2.0f;
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    (void)window; // Mark as intentionally unused

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    GApp->m_Player->ProcessMouseInput(xoffset, yoffset);
}
void resize_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    GApp->Width = width;
    GApp->Height = height;
}
void App::processInput()
{
    if(glfwGetKey(m_Window.GetGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if(!firstClick) {
            firstClick = true;
            if(glfwGetInputMode(m_Window.GetGLFWwindow(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED) 
                glfwSetInputMode(m_Window.GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else
                glfwSetInputMode(m_Window.GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    } else {
        firstClick = false;
    }
        

    /*if (glfwGetKey(m_Window.GetGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
        m_Camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(m_Window.GetGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
        m_Camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(m_Window.GetGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
        m_Camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(m_Window.GetGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
        m_Camera.ProcessKeyboard(RIGHT, deltaTime);*/
}