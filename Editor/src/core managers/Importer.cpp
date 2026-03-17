#include "Importer.h"
#include "VertexStruct.h"
#include "AssetFormats/SkeletalMeshAsset.h"

void Importer::ImportSkeletalMesh(const char* folderPath) {
    for(auto node : cachedSkeletalMeshes) {
        std::string assetName = node->mName.C_Str();
        std::string assetPath = std::string(folderPath) + "/" + assetName + ".vsa";

        std::vector<SkeletalVertex> verticies;
        std::vector<uint32_t> indicies;
        
        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = cachedScene->mMeshes[node->mMeshes[i]];
            size_t indiciesOffset = verticies.size();

            //gather all verticies
            for(unsigned int v = 0; v < mesh->mNumVertices; v++) {
                SkeletalVertex vertex;
                //get the verticies position
                vertex.pos.x = mesh->mVertices[v].x;
                vertex.pos.y = mesh->mVertices[v].y;
                vertex.pos.z = mesh->mVertices[v].z;

                //get the normals
                if(mesh->HasNormals()) {
                    vertex.normal.x = mesh->mNormals[v].x;
                    vertex.normal.y = mesh->mNormals[v].y;
                    vertex.normal.z = mesh->mNormals[v].z;
                } else {
                    vertex.normal = glm::vec3(0.0f);
                }

                //get the UVs
                if(mesh->mTextureCoords[0]) {
                    vertex.uv.x = mesh->mTextureCoords[0][v].x;
                    vertex.uv.y = mesh->mTextureCoords[0][v].y;
                } else {
                    vertex.uv = glm::vec2(0.0f);
                }

                //get the bone weights and ids
                vertex.boneIDs = glm::ivec4(-1);
                vertex.boneWeights = glm::vec4(0.0f);

                verticies.push_back(vertex);
            }

            //gather all indicies
            for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
                aiFace face = mesh->mFaces[j];
                for(unsigned int k = 0; k < face.mNumIndices; k++) {
                    indicies.push_back(face.mIndices[k] + indiciesOffset);
                }
            }
        }

        SkeletalMeshAsset skeletalMesh;

        skeletalMesh.MetaData.offset = sizeof(skeletalMesh.header) + sizeof(skeletalMesh.MetaData);
        skeletalMesh.MetaData.verticiesSize = sizeof(SkeletalVertex) * verticies.size();
        skeletalMesh.MetaData.indiciesSize = sizeof(uint32_t) * indicies.size();

        skeletalMesh.Save(assetPath.c_str());

        //reopens the file just to also write the model data, since saving a skeletal mesh only saves the metadata
        std::ofstream file(assetPath.c_str(), std::ios::binary);
        file.seekp(skeletalMesh.MetaData.offset);
        file.write(reinterpret_cast<char*>(verticies.data()), skeletalMesh.MetaData.verticiesSize);
        file.write(reinterpret_cast<char*>(indicies.data()), skeletalMesh.MetaData.indiciesSize);
        file.close();
    }
}

void Importer::TraverseModelFile(const char* path, const char* folderPath) {
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PopulateArmatureData);
    cachedScene = scene;
    ProcessNode(scene->mRootNode, scene);
    ProcessAnimations(scene);

    //temporary, remember to delete later
    ImportSkeletalMesh(folderPath);
    FreeModelCache();
}
void Importer::FreeModelCache() {
    importer.FreeScene();
    cachedStaticMeshes.clear();
    cachedSkeletalMeshes.clear();
    cachedAnimations.clear();
}

void Importer::ProcessNode(aiNode* node, const aiScene* scene) {
    if(node->mNumMeshes > 0) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
        if(mesh->HasBones()) {
            cachedSkeletalMeshes.push_back(node);
        }
        else {
            cachedStaticMeshes.push_back(node);
        }
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}
void Importer::ProcessAnimations(const aiScene* scene) {
    if(!scene->HasAnimations()) return;

    for(unsigned int i = 0; i < scene->mNumAnimations; i++) {
        cachedAnimations.push_back(scene->mAnimations[i]);
    }
}