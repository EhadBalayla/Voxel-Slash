#include "Editor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "AssetFormats/TransformAsset.h"

#include "Canvas.h"

float LastTime = 0.0f;
void RenderPrefabs(PrefabNode* node, glm::mat4 parentTrans);
void RenderCanvas(UINode* node, glm::mat4 parentTrans, int ScrWidth, int ScrHeight);


Editor::Editor() {
    GEditor = this;
}

void Editor::Init() {
    Window::InitGLFW();
    m_Window.CreateWindow("Voxel Slash Editor", Width, Height, false);
    m_Window.MakeContext();

    m_Renderer.Init();

    m_3DShader.LoadShader("Shaders/MeshShader_vert.spv", "Shaders/MeshShader_frag.spv", PipelineType::D3);
    m_2DShader.LoadShader("Shaders/UIShader_vert.spv", "Shaders/UIShader_frag.spv", PipelineType::D2);
    m_TextShader.LoadShader("Shaders/TextShader_vert.spv", "Shaders/TextShader_frag.spv", PipelineType::D2);

    GContext->SingleImagePipeline = m_2DShader.GetPipeline();
    GContext->TextPipeline = m_TextShader.GetPipeline();

    m_Editor.Init();

    proj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
}
void Editor::Loop() {
    while(!m_Window.ShouldClose()) {
        m_Window.PollEvents();
        m_Window.StartFrame();

        DeltaTime = glfwGetTime() - LastTime;
        LastTime = glfwGetTime();

        view = m_Camera.GetViewMatrix();

        m_Renderer.StartGPass();
        m_3DShader.Bind();
        RenderPrefabs(&m_Editor.GetPrefab().m_RootNode, glm::mat4(1.0f));

        if(m_Editor.GetCanvas()) {
            m_2DShader.Bind();
            m_Editor.GetCanvas()->Render(m_Renderer.GetFrameCommandBuffer(), Width, Height);
        }
        m_Renderer.EndGPass();

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
        node->m_Asset->Render(GEditor->m_Renderer.GetFrameCommandBuffer(), GEditor->m_Renderer.Get3DPipelineLayout(), GEditor->proj * GEditor->view * overall);
    }

    for(auto c : node->m_Children) {
        RenderPrefabs(c, overall);
    }
}