#include "Importer.h"

//includes of the assets
#include "AssetFormats/SkeletalMeshAsset.h"
#include "AssetFormats/TextureAsset.h"
#include "AssetFormats/FontAsset.h"

//includes of the libraries need for import (assimp is included in the Importer.h itself so i can make variables for caching etc...)
#include "stb_image.h"
#include "VertexStruct.h"
#include <ft2build.h>
#include FT_FREETYPE_H

void Importer::ImportTexture(const char* path, const char* importPath) {
    int Width, Height, Channels;
    unsigned char* pixelData = stbi_load(path, &Width, &Height, &Channels, 4);
    
    TextureAsset asset;

    asset.MetaData.Width = Width;
    asset.MetaData.Height = Height;
    asset.MetaData.Channels = Channels;

    asset.MetaData.DataOffset = sizeof(AssetHeader) + sizeof(TextureMetaData);
    asset.MetaData.DataSize = Width * Height * 4;

    asset.Save(importPath);

    std::fstream file(importPath, std::ios::binary | std::ios::in | std::ios::out); //reopening the file to patch it with the pixel data of the texture
    file.seekp(asset.MetaData.DataOffset);
    file.write(reinterpret_cast<char*>(pixelData), asset.MetaData.DataSize);
    file.close();

    stbi_image_free(pixelData);
}
void Importer::ImportFont(const char* path, const char* importPath) {
    FT_Library library;
	if (FT_Init_FreeType(&library)) {
		return;
	}

	FT_Face face;
	if (FT_New_Face(library, path, 0, &face)) {
		return;
	}

    constexpr int cellSize = 64;
	constexpr int cellLineCount = 16;
	constexpr int cellRowCount = 16;
	FT_Set_Pixel_Sizes(face, 0, cellSize);

    const int ATLAS_SIZE = cellLineCount * cellSize;
    size_t pixelBufferSize = ATLAS_SIZE * ATLAS_SIZE * 4;
    unsigned char* pixelBuffer = (unsigned char*)calloc(pixelBufferSize, 1);

    std::vector<CharInfo> Characters;

    for (unsigned char c = 0; c < 128; c++) {
		if (!FT_Load_Char(face, c, FT_LOAD_RENDER)) {

			int cellX = c % 16;
			int cellY = c / 16;

			for (int x = 0; x < face->glyph->bitmap.width; x++) {
				for (int y = 0; y < face->glyph->bitmap.rows; y++) {
					unsigned char alpha = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];

					int cursorX = cellX * cellSize + x;
					int cursorY = cellY * cellSize + y;

					pixelBuffer[(cursorY * ATLAS_SIZE + cursorX) * 4] = 255;
					pixelBuffer[(cursorY * ATLAS_SIZE + cursorX) * 4 + 1] = 255;
					pixelBuffer[(cursorY * ATLAS_SIZE + cursorX) * 4 + 2] = 255;
					pixelBuffer[(cursorY * ATLAS_SIZE + cursorX) * 4 + 3] = alpha;
				}
			}

			CharInfo ch{};
            ch.uvStart = { (cellX * cellSize) / (float)ATLAS_SIZE, (cellY * cellSize) / (float)ATLAS_SIZE };
            ch.uvOffset = { face->glyph->bitmap.width / (float)ATLAS_SIZE, face->glyph->bitmap.rows / (float)ATLAS_SIZE };
			ch.Advance = face->glyph->advance.x;
			ch.Bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			ch.Size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };

			Characters.push_back(ch);
		}
	}

    FontAsset newFont;

    newFont.MetaData.DataOffset = sizeof(AssetHeader) + sizeof(FontMetaData);
    newFont.MetaData.CharCount = Characters.size();
    newFont.MetaData.AtlasBufferSize = pixelBufferSize;

    newFont.Save(importPath);

    std::fstream file(importPath, std::ios::binary | std::ios::in | std::ios::out);
    file.seekp(newFont.MetaData.DataOffset);
    file.write(reinterpret_cast<char*>(Characters.data()), sizeof(CharInfo) * newFont.MetaData.CharCount);
    file.write(reinterpret_cast<char*>(pixelBuffer), pixelBufferSize);
    file.close();

    free(pixelBuffer);
}
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
        std::fstream file(assetPath.c_str(), std::ios::binary | std::ios::out | std::ios::in);
        file.seekp(skeletalMesh.MetaData.offset, std::ios::beg);
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