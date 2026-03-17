#include "EditorManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Editor.h"

#include <Windows.h> //specifically for the dialogue boxes
#include <ShlObj.h>
#include <filesystem>

char buff[256];
char buff2[256];
PrefabNode* cachedNode = nullptr;
bool IsRightClickOnViewport = false;

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
    if(IsRightClickOnViewport) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags = ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace", nullptr, dockspace_flags);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0, 0.0), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    if(!IsDataFolderChosen) {
        ImVec2 WindSize = ImGui::GetContentRegionAvail();
        ImVec2 MidOffset = ImVec2(WindSize.x / 2.0f, WindSize.y / 2.0f);
        
        ImVec2 topLeftPos = ImGui::GetCursorPos();
        ImVec2 MidPos = ImVec2(topLeftPos.x + MidOffset.x, topLeftPos.y + MidOffset.y);
        ImGui::SetCursorPos(MidPos);
        if(ImGui::Button("Select Folder", ImVec2(100.0f, 100.0f))) {
            BROWSEINFO bi = {0};
            bi.lpszTitle = "Select a folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            char path[MAX_PATH];
            if(SHGetPathFromIDList(pidl, path)) {
                bool HasNoVoxAssets = std::filesystem::exists(std::filesystem::path(path) / "NonVoxelAssets");
                bool HasPrefabs = std::filesystem::exists(std::filesystem::path(path) / "Prefabs");
                if(HasNoVoxAssets && HasPrefabs) {
                    DataFolder = path;
                    IsDataFolderChosen = true;
                }
            }
            CoTaskMemFree(pidl);
        }
    } else {
        ImGui::Begin("Prefab Editing", nullptr, ImGuiWindowFlags_MenuBar);

        ImGuiID prefabDockspace_id = ImGui::GetID("PrefabEditingDockspace");
        ImGui::DockSpace(prefabDockspace_id, ImVec2(0.0, 0.0), ImGuiDockNodeFlags_PassthruCentralNode);

        if(ImGui::BeginMenuBar()) {
            if(ImGui::BeginMenu("File")) {
                if(ImGui::MenuItem("Load Prefab")) {
                    char Title[] = "Load Prefab";
                    char szFileName[MAX_PATH] = "";

                    OPENFILENAME ofn;
                    ZeroMemory(&ofn, sizeof(OPENFILENAME));

                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.lpstrFilter = "Prefab Files (*.pfb)\0*.pfb\0";
                    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
                    ofn.lpstrFile = szFileName;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.hwndOwner = nullptr;
                    ofn.lpstrDefExt = "pfb";

                    if(GetOpenFileName(&ofn)) {
                        m_Prefab.Load(szFileName);
                    }
                }
                if(ImGui::MenuItem("Save Prefab")) {
                    char Title[] = "Save Prefab";
                    char szFileName[MAX_PATH] = "";

                    OPENFILENAME ofn;
                    ZeroMemory(&ofn, sizeof(OPENFILENAME));

                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.lpstrFilter = "Prefab Files (*.pfb)\0*.pfb\0";
                    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
                    ofn.lpstrFile = szFileName;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.hwndOwner = nullptr;
                    ofn.lpstrDefExt = "pfb";

                    if(GetSaveFileName(&ofn)) {
                        m_Prefab.Save(szFileName);
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();

        ImGui::Begin("Asset Editor", nullptr, ImGuiWindowFlags_MenuBar);

        ImGuiID assetDockspace_id = ImGui::GetID("AssetEditingDockspace");
        ImGui::DockSpace(assetDockspace_id, ImVec2(0.0, 0.0), ImGuiDockNodeFlags_PassthruCentralNode);

        if(ImGui::BeginMenuBar()) {

            ImGui::EndMenuBar();
        }

        ImGui::End();

        //all panels of the prefab editing
        {
            ImGui::Begin("Viewport");
            VkImageMemoryBarrier barrier{};
	        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	        barrier.image = GEditor->m_Renderer.GetColorBuffer();
	        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	        barrier.subresourceRange.baseMipLevel = 0;
	        barrier.subresourceRange.levelCount = 1;
	        barrier.subresourceRange.baseArrayLayer = 0;
	        barrier.subresourceRange.layerCount = 1;
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

            if(ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                if(!IsRightClickOnViewport) {
                    IsRightClickOnViewport = true;
                    glfwSetInputMode(GEditor->m_Window.GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    double MouseX, MouseY;
                    glfwGetCursorPos(GEditor->m_Window.GetGLFWwindow(), &MouseX, &MouseY);
                    GEditor->m_Camera.LastX = (float)MouseX;
                    GEditor->m_Camera.LastY = (float)MouseY;
                }
                if (glfwGetKey(GEditor->m_Window.GetGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
                    GEditor->m_Camera.ProcessKeyboard(FORWARD, GEditor->DeltaTime);
                if (glfwGetKey(GEditor->m_Window.GetGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
                    GEditor->m_Camera.ProcessKeyboard(BACKWARD, GEditor->DeltaTime);
                if (glfwGetKey(GEditor->m_Window.GetGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
                    GEditor->m_Camera.ProcessKeyboard(LEFT, GEditor->DeltaTime);
                if (glfwGetKey(GEditor->m_Window.GetGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
                    GEditor->m_Camera.ProcessKeyboard(RIGHT, GEditor->DeltaTime);

                double MouseX, MouseY;
                glfwGetCursorPos(GEditor->m_Window.GetGLFWwindow(), &MouseX, &MouseY);
                GEditor->m_Camera.ProcessMouseMovement((float)MouseX, (float)MouseY);
            } else {
                if(IsRightClickOnViewport) {
                    glfwSetInputMode(GEditor->m_Window.GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    IsRightClickOnViewport = false;
                }
            }

            ImGui::End();

            ImGui::Begin("Prefab Graph");
            RenderPrefabNodes(&m_Prefab.m_RootNode);
            ImGui::End();

            ImGui::Begin("Properties");
            if(selectedNode) {
                if(ImGui::InputText("Node Name: ", buff, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    selectedNode->m_Name = buff;
                }
                if(ImGui::Button("Add Child")) {
                    cachedNode = selectedNode;
                    ImGui::OpenPopup("NewNodePopup");
                }
                ImGui::InputFloat3("Position: ", reinterpret_cast<float*>(&selectedNode->pos));
                ImGui::InputFloat3("Rotation: ", reinterpret_cast<float*>(&selectedNode->rot));
                ImGui::InputFloat3("Scale: ", reinterpret_cast<float*>(&selectedNode->scale));
                if(ImGui::BeginCombo("Asset", "No Asset yet")) {

                    ImGui::EndCombo();
                }
            }
            if(ImGui::BeginPopup("NewNodePopup")) {
                ImGui::InputText("##SetNewNodeName", buff2, 256, ImGuiInputTextFlags_EnterReturnsTrue);
                if(ImGui::Button("Ok")) {
                    std::string newNodeName = buff2;
                    AddNewPrefabNode(cachedNode, newNodeName);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::End();

            ImGui::Begin("Toolbar");

            ImGui::End();
        }

        //all panels for asset importing
        {
            ImGui::Begin("Asset Browser");
            ImVec2 topLeftPos = ImGui::GetCursorPos();
            ImGui::BeginChild("AssetToolbar", ImVec2(0, 30), false, ImGuiWindowFlags_NoScrollbar);
            if(ImGui::Button("Import Asset")) {
                char Title[] = "Load Prefab";
                char szFileName[MAX_PATH] = "";
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.lpstrFilter = "All Files (*.*)\0*.fbx\0*.png\0";
                ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.hwndOwner = nullptr;
                if(GetOpenFileName(&ofn)) {
                    std::string extension = std::filesystem::path(ofn.lpstrFile).extension().string();
                    if(extension == ".fbx") {
                        m_Importer.TraverseModelFile(ofn.lpstrFile, DataFolder.c_str());
                    }
                    else if(extension == ".png") {
                    }
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Refresh")) {

            }
            ImGui::SameLine();
            if(ImGui::Button("<-")) {
            }
            ImGui::EndChild();

            ImGui::BeginChild("Asset List");
            ImGui::EndChild();

            ImGui::End();
        }
    }

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

Prefab& EditorManager::GetPrefab() {
    return m_Prefab;
}

void EditorManager::RenderPrefabNodes(PrefabNode* m_Node) {
    if(ImGui::TreeNode(m_Node->m_Name.c_str())) {
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            selectedNode = m_Node;
            strcpy(buff, selectedNode->m_Name.c_str());
        }
        for(auto n : m_Node->m_Children) {
            RenderPrefabNodes(n);
        }
        ImGui::TreePop();
    }
}

void EditorManager::RefreshAssetList() {
    if(!IsDataFolderChosen) return;

    AssetList.clear();
    for(auto& n : std::filesystem::directory_iterator(DataFolder + "/NonVoxelAssets")) {
        if(n.is_directory() || n.path().extension().string() != ".vsa") continue;

        AssetType type;
        std::ifstream file(n.path().c_str(), std::ios::binary);
        file.read(reinterpret_cast<char*>(&type), sizeof(AssetType));
        file.close();
    }
}