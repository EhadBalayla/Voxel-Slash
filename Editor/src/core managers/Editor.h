#pragma once
#include "Window.h"
#include "EditorManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Camera.h"
#include "ModInstance.h"

class Editor {
public:
    Window m_Window;
    EditorManager m_Editor;
    Renderer m_Renderer;
    Shader m_3DShader;
    Shader m_2DShader;
    Shader m_TextShader;
    ModInstance* mod = nullptr;

    Camera m_Camera;

    float DeltaTime = 0.0f;

    Editor();

    int Width = 1600, Height = 900;
    glm::mat4 proj;
    glm::mat4 view;

    void Init();
    void Loop();
    void Terminate();
};

extern Editor* GEditor;