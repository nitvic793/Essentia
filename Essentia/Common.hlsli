
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

struct DirectionalLight
{
	float3 Direction;
	float3 Color;
};

float3 CalculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.Color * NdotL + float3(0.1,0.1,0.1);
}