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

sampler BasicSampler : register(s0);

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D RoughnessTexture : register(t2);
Texture2D MetalnessTexture : register(t3);

//IBL
TextureCube skyIrradianceTexture : register(t16);
Texture2D brdfLUTTexture : register(t17);
TextureCube skyPrefilterTexture : register(t18);

float3 ToneMapFilmicALU(float3 color)
{
	color = max(0, color - 0.004f);
	color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

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

	uint i = 0;
	for (i = 0; i < DirLightCount; ++i)
	{
		finalColor += DirLightPBR(DirLights[i], normalize(normal), worldPos,
		CameraPosition, roughness, metal, texColor.rgb,
		specColor, irradiance, prefilter, brdf, 1.f);
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