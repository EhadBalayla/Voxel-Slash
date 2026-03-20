#include <unordered_map>
#include <string>
#include "AssetFormats/Asset.h"
#include "Prefab.h"

class ModInstance {
public:
    ModInstance(const char* ModPath);
    ~ModInstance();

    std::unordered_map<std::string, Asset*>& GetAllAssets();
    std::unordered_map<std::string, Prefab>& GetAllPrefabs();
private:
    std::unordered_map<std::string, Asset*> assets;
    std::unordered_map<std::string, Prefab> prefabs;
};