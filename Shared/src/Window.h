#pragma once
#include "Context.h"
#include "Swapchain.h"

struct GLFWwindow;

class Window {
public:
    static void InitGLFW();
    static void TerminateGLFW();

    //creation functions
    void CreateWindow(const char* name, int Width, int Height, bool Resizing);
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

    const int GetWindowWidth() const;
    const int GetWindowHeight() const;
    void SetWindowWidth(int Width);
    void SetWindowHeight(int Height);

    //getters
    GLFWwindow* GetGLFWwindow();
    Context& GetContext();
    Swapchain& GetSwapchain();
private:
    int Width, Height;

    GLFWwindow* m_GLFWwindow;

    Context m_Context;
    Swapchain m_Swapchain;
};

extern Window* GWindow;