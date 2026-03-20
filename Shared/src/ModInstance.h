#include <unordered_map>
#include <string>

class Asset;
class Prefab;
class Canvas;

class ModInstance {
public:
    ModInstance(const char* ModPath);
    ~ModInstance();

    std::unordered_map<std::string, Asset*>& GetAllAssets();
    std::unordered_map<std::string, Prefab*>& GetAllPrefabs();
    std::unordered_map<std::string, Canvas*>& GetAllCanvases();
private:
    std::unordered_map<std::string, Asset*> assets;
    std::unordered_map<std::string, Prefab*> prefabs;
    std::unordered_map<std::string, Canvas*> canvases;
};