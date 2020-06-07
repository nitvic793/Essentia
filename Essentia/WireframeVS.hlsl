
//Reference: https://stackoverflow.com/questions/49350004/wireframe-shader-how-to-display-quads-and-not-triangles

struct VertexInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT0;
};

struct GSInput
{
    float4 Position : SV_POSITION;
    float4 WorldPos : POSITION;
};

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};

GSInput main(VertexInput input)
{
    GSInput output;
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPos;
    output.Position = mul(float4(input.Position, 1.f), WorldViewProjection);
    return output;
}