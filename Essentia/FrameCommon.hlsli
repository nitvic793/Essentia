

struct VoxelParams
{
    float3 VoxelGridCenter;
    float VoxelRadianceDataSize; // voxel half-extent in world space units
    float VoxelRadianceDataSizeRCP; // 1.0 / voxel-half extent
    uint VoxelRadianceDataRes; // voxel grid resolution
    float VoxelRadianceDataResRCP; // 1.0 / voxel grid resolution
    uint VoxelRadianceNumCones;
    float VoxelRadianceNumConesRCP;
    float VoxelRadianceMaxDistance;
    float VoxelRadianceRayStepSize;
    uint VoxelRadianceMips;
};

struct PerFrameData
{
    float4x4 ViewProjectionTex;
    VoxelParams VoxelData;
};
