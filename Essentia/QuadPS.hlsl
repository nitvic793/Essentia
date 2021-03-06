
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D<float4>		InputTexture		: register(t0);
SamplerState			BasicSampler		: register(s0);
SamplerState			LinearWrapSampler	: register(s2);

float4 main(VertexToPixel input) : SV_TARGET
{
    return float4(InputTexture.Sample(LinearWrapSampler, input.uv).rgb, 1.0f);
}