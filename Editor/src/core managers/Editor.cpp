#include "Editor.h"
#include <glm/gtc/matrix_transform.hpp>

float LastTime = 0.0f;
void RenderPrefabs(PrefabNode* node, glm::mat4 parentTrans);

Editor::Editor() {
    GEditor = this;
}

void Editor::Init() {
    Window::InitGLFW();
    m_Window.CreateWindow("Voxel Slash Editor", Width, Height);
    m_Window.MakeContext();

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

    m_3DShader.LoadShader("Shaders/MeshShader_vert.spv", "Shaders/MeshShader_frag.spv", PipelineType::D3);

    m_Editor.Init();

    proj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
}
void Editor::Loop() {
    while(!m_Window.ShouldClose()) {
        m_Window.StartFrame();
        m_Window.PollEvents();

        DeltaTime = glfwGetTime() - LastTime;
        LastTime = glfwGetTime();

        view = m_Camera.GetViewMatrix();

        m_Renderer.StartRender();
        m_3DShader.Bind();
        RenderPrefabs(&m_Editor.GetPrefab().m_RootNode, glm::mat4(1.0f));
        m_Renderer.EndRender();

        m_Editor.Render();
        m_Window.StartFullscreenRender();
        m_Editor.Finalize();
        m_Window.EndFullscreenRender();

        m_Window.NextFrame();
    }
}
void Editor::Terminate() {
    m_Editor.Terminate();

    m_Renderer.Terminate();

    m_Window.DestroyContext();
    m_Window.DestroyWindow();
    Window::TerminateGLFW();
}

Editor* GEditor = nullptr;

void RenderPrefabs(PrefabNode* node, glm::mat4 parentTrans) {
    glm::mat4 pos = glm::translate(glm::mat4(1.0f), node->pos);

    float yaw   = glm::radians(node->rot.y);
    float pitch = glm::radians(node->rot.x);
    float roll  = glm::radians(node->rot.z);
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rot = rotY * rotX * rotZ;

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), node->scale);

    glm::mat4 overall = pos * rot * scale;
    if(node->m_Parent) overall = parentTrans * overall;

    if(node->m_Asset) {
        GEditor->m_Renderer.SetModelViewProj(GEditor->proj * GEditor->view * overall);
        vkCmdDraw(GEditor->m_Renderer.GetFrameCommandBuffer(), 36, 1, 0, 0);
    }

    for(auto c : node->m_Children) {
        RenderPrefabs(c, overall);
    }
}