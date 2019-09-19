
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

	float3 texColor = InputTexture.Sample(BasicSampler, input.uv).rgb;
	float3 color = float3(0.f, 0.f, 0.f);
	float hstep = dir.x / Width;
	float vstep = dir.y / Height;

	for (int i = 0; i < KERNEL_TAPS; ++i)
	{
		float3 lColor = InputTexture.Sample(BasicSampler, input.uv + float2(hstep * offset[i], vstep * offset[i])).rgb * kernel[i];
		float3 rColor = InputTexture.Sample(BasicSampler, input.uv - float2(hstep * offset[i], vstep * offset[i])).rgb * kernel[i];
		color = color + (lColor + rColor);
	}

	texColor = color;

	return float4(texColor, 1.f);
}