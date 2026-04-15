#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "../app.h"

#include "VertexStruct.h"


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
	VkVertexInputAttributeDescription skeletalAttributeDescriptions[6] = {};
	skeletalAttributeDescriptions[0].binding = 0;
	skeletalAttributeDescriptions[0].location = 0;
	skeletalAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	skeletalAttributeDescriptions[0].offset = offsetof(SkeletalVertex, pos);

	skeletalAttributeDescriptions[1].binding = 0;
	skeletalAttributeDescriptions[1].location = 1;
	skeletalAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	skeletalAttributeDescriptions[1].offset = offsetof(SkeletalVertex, normal);

	skeletalAttributeDescriptions[2].binding = 0;
	skeletalAttributeDescriptions[2].location = 2;
	skeletalAttributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	skeletalAttributeDescriptions[2].offset = offsetof(SkeletalVertex, tangent);

	skeletalAttributeDescriptions[3].binding = 0;
	skeletalAttributeDescriptions[3].location = 3;
	skeletalAttributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	skeletalAttributeDescriptions[3].offset = offsetof(SkeletalVertex, uv);

	skeletalAttributeDescriptions[4].binding = 0;
	skeletalAttributeDescriptions[4].location = 4;
	skeletalAttributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;
	skeletalAttributeDescriptions[4].offset = offsetof(SkeletalVertex, boneIDs);

	skeletalAttributeDescriptions[5].binding = 0;
	skeletalAttributeDescriptions[5].location = 5;
	skeletalAttributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	skeletalAttributeDescriptions[5].offset = offsetof(SkeletalVertex, boneWeights);

	VkVertexInputBindingDescription skeletalBindingDescription{};
	skeletalBindingDescription.binding = 0;
	skeletalBindingDescription.stride = sizeof(SkeletalVertex);
	skeletalBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;



	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	switch (type) {
	case PipelineType::SkeletalMesh: {
		vertexInputInfo.vertexAttributeDescriptionCount = 6;
		vertexInputInfo.pVertexAttributeDescriptions = skeletalAttributeDescriptions;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &skeletalBindingDescription;
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
	inputAssemblyInfo.topology = (type == PipelineType::Chunk || type == PipelineType::SkeletalMesh || type == PipelineType::UIShader) ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	//creating the rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = (type == PipelineType::Chunk || type == PipelineType::SkeletalMesh || type == PipelineType::UIShader) ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
	rasterizerInfo.lineWidth = 5.0f;
	rasterizerInfo.cullMode = type == PipelineType::Chunk ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_TRUE;
	rasterizerInfo.depthBiasConstantFactor = 4.0f;
	rasterizerInfo.depthBiasClamp = 0.0f;
	rasterizerInfo.depthBiasSlopeFactor = 1.5f;

	//creating the multisampler
	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//creating the color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlendAttachment.blendEnable = type == PipelineType::UIShader ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendAttachmentState colorBlendAttachment2{};
	colorBlendAttachment2.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment2.blendEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState attachments[] = { colorBlendAttachment, colorBlendAttachment2, colorBlendAttachment2, colorBlendAttachment2 };

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.attachmentCount = 4;
	colorBlendState.pAttachments = attachments;

	//creating the depth testing
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	if (type == PipelineType::Chunk || type == PipelineType::BoxOutline || type == PipelineType::SkeletalMesh) { //where we enable depth testing
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.stencilTestEnable = VK_FALSE;
	}
	else { //where we dont want depth testing
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.stencilTestEnable = VK_FALSE;
	}

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
	pipelineInfo.layout = type == PipelineType::UIShader ? GContext->GetSingleTexPPLayout() : GApp->m_ChunkRenderer.GetChunksPipelineLayout();
	pipelineInfo.renderPass = GApp->m_Renderer.GetOffscreenRenderPass();
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(GContext->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}


	vkDestroyShaderModule(GContext->GetDevice(), fragmentModule, nullptr);
	vkDestroyShaderModule(GContext->GetDevice(), vertexModule, nullptr);
}
void Shader::UnloadShader() {
	vkDestroyPipeline(GContext->GetDevice(), graphicsPipeline, nullptr);
}


void Shader::Bind() {
	vkCmdBindPipeline(GRenderer->GetFrameCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

VkPipeline* Shader::GetPipeline() {
	return &graphicsPipeline;
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

	if (vkCreateShaderModule(GApp->m_Renderer.GetDevice(), &createInfo, nullptr, &module) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create shader module");
	}

	return module;
}