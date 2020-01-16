
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer MotionBlurParams : register(b0)
{
	float2	ScreenSize;
	float	VelocityScale;
}

Texture2D<float4>		InputTexture		: register(t0);
Texture2D<float4>		VelocityTexture		: register(t1);
SamplerState			BasicSampler		: register(s0);
SamplerState			LinearWrapSampler	: register(s1);
SamplerState			PointClamSampler	: register(s2);

static const int MAX_SAMPLES = 8;

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 texelSize = 1.f / ScreenSize;
    float2 velocity = VelocityTexture.Sample(LinearWrapSampler, input.uv).xy;
	velocity *= VelocityScale;

	float speed = length(velocity / texelSize);
	int nSamples = clamp(int(speed), 1, MAX_SAMPLES);
    float3 result = InputTexture.Sample(LinearWrapSampler, input.uv).rgb;
	[unroll]
	for (int i = 1; i < nSamples; ++i)
	{
		float2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
		float2 UV = clamp(offset + input.uv, float2(0.f, 0.f), float2(1.f, 1.f));
        result += InputTexture.SampleLevel(LinearWrapSampler, UV, 0).rgb;
    }

	result /= float(nSamples);

	return float4(result, 1.0f);
}