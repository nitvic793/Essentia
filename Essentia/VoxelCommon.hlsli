struct GSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 WorldPos : POSITION0;
    float4 ShadowPos : SHADOW_POS;
};

struct GSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 WorldPos : POSITION0;
    float4 ShadowPos : SHADOW_POS;
};

struct VertexInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT0;
};
