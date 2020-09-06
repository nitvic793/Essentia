
struct VertexInput
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT0;
};

struct VertexAnimatedInput
{
    float3 Position		: POSITION;
    float2 UV			: TEXCOORD;
    float3 Normal		: NORMAL;
    float3 Tangent		: TANGENT0;
    uint4  SkinIndices	: BLENDINDICES;
    float4 SkinWeights	: BLENDWEIGHT;
};

struct PixelInput
{
	float4 Position		: SV_POSITION;
	float2 UV			: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 WorldPos		: POSITION0;
    float4 ShadowPos	: SHADOW_POS;
    float4 SSAOPos		: POSITION1;
};

float3 CalculateNormalFromSample(float3 normalSample, float2 uv, float3 normal, float3 tangent)
{
	float3 unpackedNormal = normalSample * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	return normalize(mul(unpackedNormal, TBN));
}