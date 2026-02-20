#include "Editor.h"

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
        m_Window.GetContext().MAX_FRAMES_IN_FLIGHT,
        &m_Window.GetContext().currentFrame
    );
    m_Renderer.Init();

    m_Editor.Init();
}
void Editor::Loop() {
    while(!m_Window.ShouldClose()) {
        m_Window.StartFrame();
        m_Window.PollEvents();

        m_Renderer.StartRender();
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