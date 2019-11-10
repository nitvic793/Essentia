//Credits: Daniel Holden, contact@theorangeduck.com http://theorangeduck.com/page/pure-depth-ssao

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D<float>		DepthTexture : register(t0);
SamplerState			BasicSampler : register(s0);

float3 NormalFromDepth(float depth, float2 uv)
{
	const float2 offset1 = float2(0.f, 0.001f);
	const float2 offset2 = float2(0.001f, 0.f);

	float depth1 = DepthTexture.Sample(BasicSampler, uv + offset1).r;
	float depth2 = DepthTexture.Sample(BasicSampler, uv + offset2).r;

	float3 p1 = float3(offset1, depth1 - depth);
	float3 p2 = float3(offset2, depth2 - depth);

	float3 normal = cross(p1, p2);
	normal.z = -normal.z;
	return normalize(normal);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float depth = DepthTexture.Sample(BasicSampler, input.uv).r;
	float3 normal = NormalFromDepth(depth, input.uv);
	return float4(normal, 1);
}