#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight	DirLight;
	PointLight			PointLights;
	float3				CameraPosition;
}

sampler		BasicSampler		: register(s0);

Texture2D	AlbedoTexture		: register(t0);
Texture2D	NormalTexture		: register(t1);
Texture2D	RoughnessTexture	: register(t2);
Texture2D	MetalnessTexture	: register(t3);

//IBL
TextureCube skyIrradianceTexture	: register(t16);
Texture2D	brdfLUTTexture			: register(t17);
TextureCube skyPrefilterTexture		: register(t18);

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

	float3 finalColor = AmbientPBR(DirLight, normalize(normal), worldPos,
		CameraPosition, roughness, metal, texColor.rgb,
		specColor, irradiance, prefilter, brdf, 1.f);

	finalColor += DirLightPBR(DirLight, normalize(normal), worldPos,
		CameraPosition, roughness, metal, texColor.rgb,
		specColor, irradiance, prefilter, brdf, 1.f);

	finalColor += PointLightPBR(PointLights, normalize(normal), worldPos, CameraPosition, roughness, metal, texColor.rgb, specColor, irradiance);

	float3 light = CalculateDirectionalLight(normal, DirLight) + CalculatePointLight(normal, CameraPosition, input.WorldPos, PointLights);
	
	clip(texColor.a - 0.2f);
	

	float3 output = finalColor;
	output = lerp(output, pow(abs(output), 1.f / 2.2f), 0.4f); 

	return float4(output, 1.0f);
}