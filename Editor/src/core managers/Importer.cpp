#include "Importer.h"

void Importer::TraverseModelFile(const char* path) {
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PopulateArmatureData);

}
void Importer::FreeModelCache() {
    importer.FreeScene();
}