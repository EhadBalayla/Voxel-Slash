#include "app.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <stdexcept>

void DebugUI::Init() {
    CreateVulkanResources();



    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    
    ImGui::StyleColorsDark();

    
    ImGui_ImplGlfw_InitForVulkan(GApp->m_Window.GetGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_0;
    initInfo.Instance = GApp->m_Window.GetContext().GetInstance();
    initInfo.PhysicalDevice = GApp->m_Window.GetContext().GetPhysicalDevice();
    initInfo.Device = GApp->m_Window.GetContext().GetDevice();
    initInfo.Queue = GApp->m_Window.GetContext().GetGraphicsQueue();
    initInfo.QueueFamily = GApp->m_Window.GetContext().GetGraphicsFamily();
    initInfo.MinImageCount = GApp->m_Window.GetContext().MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount = GApp->m_Window.GetSwapchain().swapchainImages.size();
    initInfo.DescriptorPool = debugUIPool;
    initInfo.PipelineInfoMain.RenderPass = GApp->m_Window.GetSwapchain().swapchainRenderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&initInfo);
}
float x = 0.0f, y = 0.0f, z = 0.0f;
void DebugUI::RenderDebugUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Menu");
    ImGui::Checkbox("Show Chunk Borders", &GApp->showChunkBorders);
    ImGui::Checkbox("Load Chunks?", &GApp->LoadChunks);
    ImGui::Checkbox("Do Physics?", &GApp->DoPhysics);
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", GApp->m_Player->Position.x, GApp->m_Player->Position.y, GApp->m_Player->Position.z);
    ImGui::Text("FPS: %.1f", 1.0f / GApp->deltaTime);
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD0: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(0).size())).c_str());
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD1: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(1).size())).c_str());
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD2: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(2).size())).c_str());
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD3: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(3).size())).c_str());
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD4: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(4).size())).c_str());
    ImGui::TextUnformatted(std::string("Num Of Chunks LOD5: " + std::to_string(GApp->m_World->GetChunkManager().GetChunkProvider().GetAllChunks(5).size())).c_str());

    ImGui::SetNextItemWidth(75.0f);
    ImGui::InputFloat("##TeleportX", &x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75.0f);
    ImGui::InputFloat("##TeleportY", &y);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75.0f);
    ImGui::InputFloat("##TeleportZ", &z);
    ImGui::SameLine();
    if(ImGui::Button("Teleport")) {
        GApp->m_Player->Position = glm::vec3(x, y, z);
    }


    ImGui::SliderFloat("##FOV", &GApp->FOV, 30.0f, 180.0f);

    ImGui::Separator();
    if(ImGui::Button("Back To Menu")) {
        GApp->waitingFrames = 1;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GApp->m_Renderer.GetFrameCommandBuffer());
}
void DebugUI::RenderMenuDebugUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Menu");
    
    if(ImGui::Button("start")) {
        GApp->state = GameState::InGame;
        GApp->waitingFrames = 0;
        GApp->m_Player->Position = glm::vec3(10.0f, 11.0f, 10.0f);
        GApp->m_World = new World();
        GApp->m_World->GetChunkManager().UpdateChunks();
    }
    ImGui::SliderInt("Max LODs", &GApp->MaxLODLevel, 1, 6);
    ImGui::SliderInt("Render Distance", &GApp->RenderDistance, 3, 12);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GApp->m_Window.GetContext().GetCommandBuffers()[GApp->m_Window.GetContext().currentFrame]);
}
void DebugUI::Terminate() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}



void DebugUI::CreateVulkanResources() {
    VkDescriptorPoolSize poolSize{};
    poolSize.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.poolSizeCount = 1;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;

    if(vkCreateDescriptorPool(GApp->m_Window.GetContext().GetDevice(), &poolInfo, nullptr, &debugUIPool) != VK_SUCCESS) {
        throw std::runtime_error("couldn't create the descriptor pool for the debug UI");
    }
}