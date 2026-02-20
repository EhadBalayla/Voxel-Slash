#pragma once
#include <GLFW/glfw3.h>
#include "Context.h"
#include "Swapchain.h"

class Window {
public:
    static void InitGLFW();
    static void TerminateGLFW();

    //creation functions
    void CreateWindow(const char* name, int Width, int Height);
    void MakeContext();

    //destruction functions
    void DestroyContext();
    void DestroyWindow();

    //loop functions
    void StartFrame();
    void NextFrame();
    void StartFullscreenRender();
    void EndFullscreenRender();
    void PollEvents();
    bool ShouldClose();



    //getters
    GLFWwindow* GetGLFWwindow();
    Context& GetContext();
    Swapchain& GetSwapchain();
private:
    GLFWwindow* m_GLFWwindow;

    Context m_Context;
    Swapchain m_Swapchain;
};