#include "Window.h"
#include "app.h"
#include <iostream>

void Window::InitGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

void Window::TerminateGLFW() {
    glfwTerminate();
}





void Window::CreateWindow(const char* name, int Width, int Height) {
    m_GLFWwindow = glfwCreateWindow(Width, Height, name, NULL, NULL);

    if (!m_GLFWwindow){
        glfwTerminate();
        exit(0);
    }

	glfwSwapInterval(0);
}
void Window::MakeContext() {
    //start the GPU initialization
    m_Context.InitGPU(m_GLFWwindow);

    GApp->m_Renderer.SetHandles(
		m_Context.GetInstance(), 
		m_Context.GetDebugMessenger(), 
		m_Context.GetPhysicalDevice(),
		m_Context.GetDevice(), 
		m_Context.GetGraphicsQueue(), 
		m_Context.GetPresentQueue(), 
		m_Context.GetSurface(), 
		m_Context.GetGraphicsFamily(), 
		m_Context.GetPresentFamily(),
        m_Context.MAX_FRAMES_IN_FLIGHT);

    //start the swapchain
    m_Swapchain.context = &m_Context;
	m_Swapchain.maxFramesInFlight = m_Context.MAX_FRAMES_IN_FLIGHT;
	m_Swapchain.window = m_GLFWwindow;
	m_Swapchain.Create();

}


void Window::DestroyContext() {
	m_Swapchain.Delete();
	
	m_Context.TerminateGPU();
}
void Window::DestroyWindow() {
    glfwDestroyWindow(m_GLFWwindow);    
}


void Window::StartFrame() {
    vkWaitForFences(m_Context.GetDevice(), 1, &m_Swapchain.inFlightFences[GApp->m_Renderer.CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_Context.GetDevice(), 1, &m_Swapchain.inFlightFences[GApp->m_Renderer.CurrentFrame]);

    vkAcquireNextImageKHR(m_Context.GetDevice(), m_Swapchain.swapchain, UINT64_MAX, m_Swapchain.imageAvailableSemaphores[GApp->m_Renderer.CurrentFrame], VK_NULL_HANDLE, &m_Swapchain.imageIndex);

    vkResetCommandBuffer(GApp->m_Renderer.GetFrameCommandBuffer(), 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(GApp->m_Renderer.GetFrameCommandBuffer(), &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to start current frame's command buffer");
    }



	uint32_t clearValuesCount = 2;
	VkClearValue clearValues[] = {{0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_Swapchain.swapchainRenderPass;
	renderPassInfo.framebuffer = m_Swapchain.swapChainFramebuffers[m_Swapchain.imageIndex];
	renderPassInfo.clearValueCount = clearValuesCount;
	renderPassInfo.pClearValues = clearValues;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_Swapchain.swapchainImageExtent;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = m_Swapchain.swapchainImageExtent.width;
	viewport.height = m_Swapchain.swapchainImageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(GApp->m_Renderer.GetFrameCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_Swapchain.swapchainImageExtent;
	vkCmdSetScissor(GApp->m_Renderer.GetFrameCommandBuffer(), 0, 1, &scissor);

	vkCmdBeginRenderPass(GApp->m_Renderer.GetFrameCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void Window::NextFrame() {
	vkCmdEndRenderPass(GApp->m_Renderer.GetFrameCommandBuffer());

    if (vkEndCommandBuffer(GApp->m_Renderer.GetFrameCommandBuffer()) != VK_SUCCESS) {
			throw std::runtime_error("couldn't end one of the rendering command buffers");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_Swapchain.imageAvailableSemaphores[GApp->m_Renderer.CurrentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GApp->m_Renderer.GetFrameCommandBuffer();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_Swapchain.renderingFinishedSemaphores[m_Swapchain.imageIndex];

	if (vkQueueSubmit(GApp->m_Renderer.GetGraphicsQueue(), 1, &submitInfo, m_Swapchain.inFlightFences[GApp->m_Renderer.CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer");
	}
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_Swapchain.renderingFinishedSemaphores[m_Swapchain.imageIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain.swapchain;
	presentInfo.pImageIndices = &m_Swapchain.imageIndex;

	vkQueuePresentKHR(GApp->m_Renderer.GetPresentQueue(), &presentInfo);
	
	GApp->m_Renderer.CurrentFrame = (GApp->m_Renderer.CurrentFrame + 1) % m_Context.MAX_FRAMES_IN_FLIGHT;
}
void Window::PollEvents() {
    glfwPollEvents();
}
bool Window::ShouldClose() {
    return glfwWindowShouldClose(m_GLFWwindow);
}





GLFWwindow* Window::GetGLFWwindow() {
    return m_GLFWwindow;
}
Context& Window::GetContext() {
	return m_Context;
}
Swapchain& Window::GetSwapchain() {
    return m_Swapchain;
}