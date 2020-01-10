#include "Common.hlsli"

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};


PixelInput main(VertexInput input)
{
    PixelInput output;
	
    output.Position = mul(float4(input.Position, 1.f), WorldViewProjection);
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.UV = input.UV;
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));;
    output.WorldPos = mul(float4(input.Position, 1.0f), World).xyz;
    output.ShadowPos = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}