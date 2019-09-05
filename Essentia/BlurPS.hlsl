
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D		InputTexture : register(t0);
SamplerState	BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(InputTexture.Sample(BasicSampler, input.uv).rgb, 1.0f);
}