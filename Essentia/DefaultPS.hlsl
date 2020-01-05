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

SamplerState BasicSampler   : register(s0);
SamplerComparisonState ShadowSampler  : register(s1);

Texture2D AlbedoTexture     : register(t0);
Texture2D NormalTexture     : register(t1);
Texture2D RoughnessTexture  : register(t2);
Texture2D MetalnessTexture  : register(t3);

//Shadow Bufer
Texture2D ShadowMapDirLight : register(t8);

//IBL
TextureCube skyIrradianceTexture    : register(t16);
Texture2D brdfLUTTexture            : register(t17);
TextureCube skyPrefilterTexture     : register(t18);

float3 ToneMapFilmicALU(float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
    return color;
	// result has 1/2.2 baked in
    return pow(color, 2.2f);
}

float3 PrefilteredColor(float3 viewDir, float3 normal, float roughness)
{
    const float MAX_REF_LOD = 8.0f; // (mip level - 1) Mip Levels = 9
    float3 R = reflect(-viewDir, normal);
    return skyPrefilterTexture.SampleLevel(BasicSampler, R, roughness * MAX_REF_LOD).rgb;
}

float2 BrdfLUT(float3 normal, float3 viewDir, float roughness)
{
    float NdotV = dot(normal, viewDir);
    NdotV = max(NdotV, 0.0f);
    float2 uv = float2(NdotV, roughness);
    return brdfLUTTexture.Sample(BasicSampler, uv).rg;
}

float ShadowAmount(float4 shadowPos)
{
    float2 shadowUV = shadowPos.xy / shadowPos.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;
    float depthFromLight = shadowPos.z / shadowPos.w;
    float shadowAmount = ShadowMapDirLight.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);
    return shadowAmount;
}

float SampleShadowMap(float2 uv, float2 offsetUV, float2 shadowSizeInv, float depth)
{
    float2 sUV = uv + offsetUV * shadowSizeInv;
    return ShadowMapDirLight.SampleCmpLevelZero(ShadowSampler, sUV, depth);
}

float SampleShadowMapOptimizedPCF(float4 shadowPos)
{
    float2 shadowMapSize = 4096; // TODO: send through CB
    float lightDepth = shadowPos.z / shadowPos.w;
    float2 uv = (shadowPos.xy / shadowPos.w * 0.5f + 0.5f);
    uv.y = 1.f - uv.y;
    uv = uv * shadowMapSize; //1 unit - 1 texel

    float2 shadowMapSizeInv = 1.0 / shadowMapSize;

    float2 base_uv;
    base_uv.x = floor(uv.x + 0.5);
    base_uv.y = floor(uv.y + 0.5);

    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);

    base_uv -= float2(0.5, 0.5);
    base_uv *= shadowMapSizeInv;


    float uw0 = (4 - 3 * s);
    float uw1 = 7;
    float uw2 = (1 + 3 * s);

    float u0 = (3 - 2 * s) / uw0 - 2;
    float u1 = (3 + s) / uw1;
    float u2 = s / uw2 + 2;

    float vw0 = (4 - 3 * t);
    float vw1 = 7;
    float vw2 = (1 + 3 * t);

    float v0 = (3 - 2 * t) / vw0 - 2;
    float v1 = (3 + t) / vw1;
    float v2 = t / vw2 + 2;


    float sum = 0;
    sum += uw0 * vw0 * SampleShadowMap(base_uv, float2(u0, v0), shadowMapSizeInv, lightDepth);
    sum += uw1 * vw0 * SampleShadowMap(base_uv, float2(u1, v0), shadowMapSizeInv, lightDepth);
    sum += uw2 * vw0 * SampleShadowMap(base_uv, float2(u2, v0), shadowMapSizeInv, lightDepth);

    sum += uw0 * vw1 * SampleShadowMap(base_uv, float2(u0, v1), shadowMapSizeInv, lightDepth);
    sum += uw1 * vw1 * SampleShadowMap(base_uv, float2(u1, v1), shadowMapSizeInv, lightDepth);
    sum += uw2 * vw1 * SampleShadowMap(base_uv, float2(u2, v1), shadowMapSizeInv, lightDepth);

    sum += uw0 * vw2 * SampleShadowMap(base_uv, float2(u0, v2), shadowMapSizeInv, lightDepth);
    sum += uw1 * vw2 * SampleShadowMap(base_uv, float2(u1, v2), shadowMapSizeInv, lightDepth);
    sum += uw2 * vw2 * SampleShadowMap(base_uv, float2(u2, v2), shadowMapSizeInv, lightDepth);

    return sum * 1.0f / 144;
}

float4 main(PixelInput input) : SV_TARGET
{
    float roughness = RoughnessTexture.Sample(BasicSampler, input.UV).r;
    float metal = MetalnessTexture.Sample(BasicSampler, input.UV).r;
    float3 worldPos = input.WorldPos;
    float4 texColor = AlbedoTexture.Sample(BasicSampler, input.UV);
    float3 color = texColor.rgb;
    float3 normalSample = NormalTexture.Sample(BasicSampler, input.UV).xyz;
    float3 normal = CalculateNormalFromSample(normalSample, input.UV, input.Normal, input.Tangent);

    float3 viewDir = normalize(CameraPosition - worldPos);
    float3 prefilter = PrefilteredColor(viewDir, normal, roughness);
    float2 brdf = BrdfLUT(normal, viewDir, roughness);

    float3 specColor = lerp(F0_NON_METAL.rrr, texColor.rgb, metal);
    float3 irradiance = skyIrradianceTexture.Sample(BasicSampler, normal).rgb;

    float3 finalColor = AmbientPBR(DirLights[CPrimaryDirLight], normalize(normal), worldPos,
		CameraPosition, roughness, metal, texColor.rgb,
		specColor, irradiance, prefilter, brdf, 1.f);
    
    float shadowAmount = SampleShadowMapOptimizedPCF(input.ShadowPos);

    uint i = 0;
    for (i = 0; i < DirLightCount; ++i)
    {
        finalColor += DirLightPBR(DirLights[i], normalize(normal), worldPos,
		CameraPosition, roughness, metal, texColor.rgb,
		specColor, irradiance, prefilter, brdf, shadowAmount);
    }

    for (i = 0; i < PointLightCount; ++i)
    {
        finalColor += PointLightPBR(PointLights[i], normalize(normal), worldPos, CameraPosition, roughness, metal, texColor.rgb, specColor, irradiance);
    }

    for (i = 0; i < SpotLightCount; ++i)
    {
        finalColor += SpotLightPBR(SpotLights[i], normalize(normal), worldPos, CameraPosition, roughness, metal, texColor.rgb, specColor);
    }
	
    clip(texColor.a - 0.2f);

    float3 output = finalColor;
    return float4(output, 1.0f);
}