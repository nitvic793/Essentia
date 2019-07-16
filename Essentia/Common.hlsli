
struct VertexInput
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
};

struct PixelInput
{
	float4 Position	: SV_POSITION;
	float2 UV		: TEXCOORD;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
};