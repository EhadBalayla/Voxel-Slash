#include "Asset.h"
#include <fstream>

void Asset::Load(const char* path) {
    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    Deserialize(file);
    file.close();
}
void Asset::Save(const char* path) {
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    Serialize(file);
    file.close();
}