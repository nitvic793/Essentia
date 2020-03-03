
#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer LightBuffer : register(b0)
{
    DirectionalLight DirLights[CMaxDirLights];
    PointLight PointLights[CMaxPointLights];
    SpotLight SpotLights[CMaxSpotLights];
    float3 CameraPosition;
    float Padding1;
    uint DirLightCount;
    uint PointLightCount;
    uint SpotLightCount;
    float Padding2;
}

float4 main(PixelInput input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}