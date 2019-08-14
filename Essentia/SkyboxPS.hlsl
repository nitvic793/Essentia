
TextureCube Sky				: register(t0);
SamplerState BasicSampler	: register(s0);

struct VertexToPixelSky
{
	float4 Position		: SV_POSITION;
	float3 UVW			: TEXCOORD;
};

// Entry point for this pixel shader
float4 main(VertexToPixelSky input) : SV_TARGET
{
	float3 finalColor = Sky.Sample(BasicSampler, input.UVW).rgb;
	//finalColor = finalColor / (finalColor + float3(1.f, 1.f, 1.f));
	float3 gammaCorrect = lerp(finalColor, pow(finalColor, 1.0 / 2.2), 0.8f);
	return float4(finalColor, 0.f);
}