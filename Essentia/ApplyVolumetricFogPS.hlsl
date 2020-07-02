
struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D<float4> InputTexture  : register(t0);
Texture2D<float4> FogTexture    : register(t1);

SamplerState BasicSampler       : register(s0);
SamplerState LinearWrapSampler  : register(s2);

float4 main(VertexToPixel input) : SV_TARGET
{
    float3 fog = FogTexture.Sample(LinearWrapSampler, input.uv).rgb ;
    float3 inputColor = InputTexture.Sample(LinearWrapSampler, input.uv).rgb;
    return float4(inputColor + fog * 3.f, 1.0f);
}