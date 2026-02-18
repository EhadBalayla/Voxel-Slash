#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "../app.h"


std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(const std::vector<char>& code);



void Shader::LoadShader(const char* vertexPath, const char* fragmentPath, PipelineType type) {
    //loading the vertex and fragment shader
	auto vertexShaderCode = readFile(vertexPath);
	auto fragmentShaderCode = readFile(fragmentPath);

	VkShaderModule vertexModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentModule = createShaderModule(fragmentShaderCode);

	//creating the vertex and fragment shader stages
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//creating the dynamic states
	uint32_t dynamicStatesCount = 2;
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = dynamicStatesCount;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	//creating the viewport and scissors for the dynamic states
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1; // must be >= 1
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	//creating the vertex input
    VkVertexInputBindingDescription ChunkBindingDescription{};
    ChunkBindingDescription.binding = 0;
    ChunkBindingDescription.stride = sizeof(uint32_t);
    ChunkBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription ChunkAttributeDescription{};
    ChunkAttributeDescription.binding = 0;
    ChunkAttributeDescription.location = 0;
    ChunkAttributeDescription.format = VK_FORMAT_R32_UINT;
    ChunkAttributeDescription.offset = 0;



	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	switch (type) {
	case PipelineType::Chunk: {
		vertexInputInfo.vertexAttributeDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = &ChunkAttributeDescription;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &ChunkBindingDescription;
		break;
	}
	default: {
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		break;
	}
	}

	//creating the input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = type == PipelineType::Chunk ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	//creating the rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = type == PipelineType::Chunk ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
	rasterizerInfo.lineWidth = 5.0f;
	rasterizerInfo.cullMode = type == PipelineType::Chunk ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor = 0.0f;
	rasterizerInfo.depthBiasClamp = 0.0f;
	rasterizerInfo.depthBiasSlopeFactor = 0.0f;

	//creating the multisampler
	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//creating the color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState attachments[] = { colorBlendAttachment };

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = attachments;

	//creating the depth testing
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//if (type == PipelineType::Surface || type == PipelineType::Skeletal) { //where we enable depth testing
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.stencilTestEnable = VK_FALSE;
	//}
	/*{ //where we dont want depth testing
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.stencilTestEnable = VK_FALSE;
	}*/

	//creating the graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.layout = App::Get()->m_Renderer.GetChunksPipelineLayout();
	pipelineInfo.renderPass = App::Get()->m_Window.GetSwapchain().swapchainRenderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(App::Get()->m_Renderer.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}


	vkDestroyShaderModule(App::Get()->m_Renderer.GetDevice(), fragmentModule, nullptr);
	vkDestroyShaderModule(App::Get()->m_Renderer.GetDevice(), vertexModule, nullptr);
}
void Shader::UnloadShader() {
	vkDestroyPipeline(App::Get()->m_Renderer.GetDevice(), graphicsPipeline, nullptr);
}


void Shader::Bind() {
	vkCmdBindPipeline(App::Get()->m_Renderer.GetFrameCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}


std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("couldn't open file for loading shader module");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
VkShaderModule createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule module;

	if (vkCreateShaderModule(App::Get()->m_Renderer.GetDevice(), &createInfo, nullptr, &module) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create shader module");
	}

	return module;
}