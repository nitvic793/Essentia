
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer BlurParams
{
	float2	Direction; // 0 - Horizontal, 1 - Vertical
	float	Width;
	float	Height;
};

static const float offset[] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };

Texture2D			InputTexture : register(t0);
Texture2D<float>	DepthTexture : register(t1);
SamplerState		BasicSampler : register(s0);
SamplerState		LinearWrapSampler : register(s2);
SamplerState		PontClampSampler : register(s3);
SamplerState		LinearClampSampler : register(s4);

float4 main(VertexToPixel input) : SV_TARGET
{
	const int KERNEL_TAPS = 6;
	float kernel[KERNEL_TAPS + 1];
	kernel[6] = 0.00000000000000;  // Weight applied to outside-radius values
	kernel[5] = 0.04153263993208;
	kernel[4] = 0.06352050813141;
	kernel[3] = 0.08822292796029;
	kernel[2] = 0.11143948794984;
	kernel[1] = 0.12815541114232;
	kernel[0] = 0.13425804976814;
	float2 dir = Direction;

    float3 texColor = InputTexture.Sample(LinearClampSampler, input.uv).rgb;
	float3 color = float3(0.f, 0.f, 0.f);
	float hstep = dir.x / Width;
	float vstep = dir.y / Height;
	float2 texelSize = dir / float2(Width, Height);

	for (int i = 0; i < KERNEL_TAPS; ++i)
	{
		float2 step = texelSize * offset[i];
		float2 luv = clamp(input.uv + step, float2(0.f, 0.f), float2(1.f, 1.f));
		float2 ruv = clamp(input.uv - step, float2(0.f, 0.f), float2(1.f, 1.f));
        float3 lColor = InputTexture.Sample(LinearClampSampler, luv).rgb * kernel[i];
        float3 rColor = InputTexture.Sample(LinearClampSampler, ruv).rgb * kernel[i];
		color = color + (lColor + rColor);
	}

	texColor = color;

	return float4(texColor, 1.f);
}