
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D<float4>		InputTexture : register(t0);
SamplerState			BasicSampler : register(s0);

float3 ToneMapFilmicALU(float3 color)
{
	color = max(0, color - 0.004f);
	color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

	// result has 1/2.2 baked in
	return pow(color, 2.2f);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 color = InputTexture.Sample(BasicSampler, input.uv).rgb;
	color = ToneMapFilmicALU(color);
	return float4(saturate(color), 1.0f);
}