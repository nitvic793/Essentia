

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

// All per frame data
struct PerFrameData
{
    float4x4 ViewProjectionTex;
    VoxelParams VoxelData;
    float4x4 ShadowView; // Global Directional Light Shadow View Matrix
    float4x4 ShadowProjection; // Global Directional Light Shadow Projection Matrix
    float4x4 CamView; // Main Camera View Matrix
    float4x4 CamProjection; // Main Camera Projection Matrix
    float4x4 CamInvView; // Main Camera Inverse View Matrix  
    float4x4 CamInvProjection; // Main Camera Inverse Projection Matrix
    float2 ScreenSize; // Screen Width and Height
};

struct VoxelType
{
    uint ColorMask;
    uint NormalMask;
};

static const float __hdrRange = 10.0f;

// Encode HDR color to a 32 bit uint
// Alpha is 1 bit + 7 bit HDR remapping
uint EncodeColor(in float4 color)
{
	// normalize color to LDR
    float hdr = length(color.rgb);
    color.rgb /= hdr;

	// encode LDR color and HDR range
    uint3 iColor = uint3(color.rgb * 255.0f);
    uint iHDR = (uint) (saturate(hdr / __hdrRange) * 127);
    uint colorMask = (iHDR << 24u) | (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;

	// encode alpha into highest bit
    uint iAlpha = (color.a > 0 ? 1u : 0u);
    colorMask |= iAlpha << 31u;

    return colorMask;
}

// Decode 32 bit uint into HDR color with 1 bit alpha
float4 DecodeColor(in uint colorMask)
{
    float hdr;
    float4 color;

    hdr = (colorMask >> 24u) & 0x0000007f;
    color.r = (colorMask >> 16u) & 0x000000ff;
    color.g = (colorMask >> 8u) & 0x000000ff;
    color.b = colorMask & 0x000000ff;

    hdr /= 127.0f;
    color.rgb /= 255.0f;

    color.rgb *= hdr * __hdrRange;

    color.a = (colorMask >> 31u) & 0x00000001;

    return color;
}

// Encode specified normal (normalized) into an unsigned integer. Each axis of
// the normal is encoded into 9 bits (1 for the sign/ 8 for the value).
uint EncodeNormal(in float3 normal)
{
    int3 iNormal = int3(normal * 255.0f);
    uint3 iNormalSigns;
    iNormalSigns.x = (iNormal.x >> 5) & 0x04000000;
    iNormalSigns.y = (iNormal.y >> 14) & 0x00020000;
    iNormalSigns.z = (iNormal.z >> 23) & 0x00000100;
    iNormal = abs(iNormal);
    uint normalMask = iNormalSigns.x | (iNormal.x << 18) | iNormalSigns.y | (iNormal.y << 9) | iNormalSigns.z | iNormal.z;
    return normalMask;
}

// Decode specified mask into a float3 normal (normalized).
float3 DecodeNormal(in uint normalMask)
{
    int3 iNormal;
    iNormal.x = (normalMask >> 18) & 0x000000ff;
    iNormal.y = (normalMask >> 9) & 0x000000ff;
    iNormal.z = normalMask & 0x000000ff;
    int3 iNormalSigns;
    iNormalSigns.x = (normalMask >> 25) & 0x00000002;
    iNormalSigns.y = (normalMask >> 16) & 0x00000002;
    iNormalSigns.z = (normalMask >> 7) & 0x00000002;
    iNormalSigns = 1 - iNormalSigns;
    float3 normal = float3(iNormal) / 255.0f;
    normal *= iNormalSigns;
    return normal;
}

// flattened array index to 3D array index
inline uint3 UnFlatten3D(uint idx, uint3 dim)
{
    const uint z = idx / (dim.x * dim.y);
    idx -= (z * dim.x * dim.y);
    const uint y = idx / dim.x;
    const uint x = idx % dim.x;
    return uint3(x, y, z);
}

// 3D array index to flattened 1D array index
inline uint Flatten3D(uint3 coord, uint3 dim)
{
    return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}