
TextureCube		Sky					: register(t0);
SamplerState	BasicSampler		: register(s0);
SamplerState	LinearWrapSampler	: register(s0);

struct VertexToPixelSky
{
	float4 Position		: SV_POSITION;
	float3 UVW			: TEXCOORD;
};

float4 main(VertexToPixelSky input) : SV_TARGET
{
    float3 finalColor = Sky.Sample(LinearWrapSampler, input.UVW).rgb;
	return float4(finalColor, 0.f);
}