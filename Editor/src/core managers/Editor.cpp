#include "Editor.h"

Editor::Editor() {
    GEditor = this;
}

void Editor::Init() {
    Window::InitGLFW();
    m_Window.CreateWindow("Voxel Slash Editor", Width, Height);
    m_Window.MakeContext();
}
void Editor::Loop() {
    while(!m_Window.ShouldClose()) {
        m_Window.StartFrame();
        m_Window.PollEvents();


        m_Window.NextFrame();
    }
}
void Editor::Terminate() {
    m_Window.DestroyContext();
    m_Window.DestroyWindow();
    Window::TerminateGLFW();
}

Editor* GEditor = nullptr;