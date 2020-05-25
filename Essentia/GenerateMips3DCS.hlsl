
cbuffer MipGenData : register(b0)
{
    uint3 OutputResolution;
}

Texture3D<float> input : register(t0);
RWTexture3D<float> output : register(u0);

SamplerState LinearWrapSampler : register(s0);

static const int CMipBlockSize = 4;

[numthreads(CMipBlockSize, CMipBlockSize, CMipBlockSize)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 TexelSize = 1.f / (float3)OutputResolution;
    if (DTid.x < OutputResolution.x && DTid.y < OutputResolution.y && DTid.z < OutputResolution.z)
    {
        output[DTid] = input.SampleLevel(LinearWrapSampler, ((float3) DTid + 0.5f) * TexelSize, 0);
    }
}