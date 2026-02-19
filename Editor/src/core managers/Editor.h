#pragma once
#include "Window.h"
#include "Rendering/Renderer.h"

class Editor {
public:
    Window m_Window;
    Renderer m_Renderer;

    Editor();

    int Width = 1600, Height = 900;

    void Init();
    void Loop();
    void Terminate();
};

extern Editor* GEditor;