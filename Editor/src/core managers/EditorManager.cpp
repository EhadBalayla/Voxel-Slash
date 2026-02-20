#include "EditorManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Editor.h"

void EditorManager::Init() {
    VkDescriptorPoolSize poolSize{};
    poolSize.descriptorCount = 50;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.poolSizeCount = 1;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 50;

    if(vkCreateDescriptorPool(GEditor->m_Window.GetContext().GetDevice(), &poolInfo, nullptr, &editorPool) != VK_SUCCESS) {
        throw std::runtime_error("couldn't create the descriptor pool for the debug UI");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    ImGui_ImplGlfw_InitForVulkan(GEditor->m_Window.GetGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_0;
    initInfo.Instance = GEditor->m_Window.GetContext().GetInstance();
    initInfo.PhysicalDevice = GEditor->m_Window.GetContext().GetPhysicalDevice();
    initInfo.Device = GEditor->m_Window.GetContext().GetDevice();
    initInfo.Queue = GEditor->m_Window.GetContext().GetGraphicsQueue();
    initInfo.QueueFamily = GEditor->m_Window.GetContext().GetGraphicsFamily();
    initInfo.MinImageCount = GEditor->m_Window.GetContext().MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount = GEditor->m_Window.GetSwapchain().swapchainImages.size();
    initInfo.DescriptorPool = editorPool;
    initInfo.PipelineInfoMain.RenderPass = GEditor->m_Window.GetSwapchain().swapchainRenderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&initInfo);

    viewportBuffers.resize(GEditor->m_Window.GetContext().MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < GEditor->m_Window.GetContext().MAX_FRAMES_IN_FLIGHT; i++) {
        viewportBuffers[i] = (ImTextureID)(ImGui_ImplVulkan_AddTexture(GEditor->m_Renderer.GetSampler(), GEditor->m_Renderer.GetColorBufferViews()[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    }
}
void EditorManager::Render() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace", nullptr, window_flags);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0, 0.0), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    ImGui::Begin("Viewport");
        VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // from rendering
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // for ImGui sampling
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = GEditor->m_Renderer.GetColorBuffer(); // your framebuffer color VkImage
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		// Access masks: writing to color attachment -> reading in shader
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		vkCmdPipelineBarrier(
			GEditor->m_Renderer.GetFrameCommandBuffer(),
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImGui::Image(viewportBuffers[*GEditor->m_Renderer.CurrentFrame], windowSize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    ImGui::Begin("Debug Menu2");

    ImGui::End();

    ImGui::Begin("Debug Menu3");

    ImGui::End();

    ImGui::Begin("Debug Menu4");

    ImGui::End();

    ImGui::Render();
}
void EditorManager::Finalize() {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GEditor->m_Renderer.GetFrameCommandBuffer());
}
void EditorManager::Terminate() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(GEditor->m_Window.GetContext().GetDevice(), editorPool, nullptr);
}