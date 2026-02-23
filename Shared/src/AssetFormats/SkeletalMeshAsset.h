#include "Asset.h"
#include "../MeshBuffer.h"

struct SkeletalMeshMetadata {
    size_t offset;
    size_t verticiesSize;
    size_t indiciesSize;
};

class SkeletalMeshAsset : public Asset {
public:
    SkeletalMeshAsset();
    ~SkeletalMeshAsset();

    SkeletalMeshMetadata MetaData;
protected:
    void Deserialize(std::ifstream& file) override;
    void Serialize(std::ofstream& file) override;
private:
    MeshBuffer mesh;
    bool CreatedMesh = false;
};