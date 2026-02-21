#include "Asset.h"

class SkeletalMeshAsset : public Asset {
public:
    SkeletalMeshAsset();

    void Load() override;
    void Save() override;
};