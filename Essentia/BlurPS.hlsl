
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer BlurParams
{
	int		Direction; // 0 - Horizontal, 1 - Vertical
	float	Width;
	float	Height;
};

static const float offset[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
static const float weight[] = {
  0.2270270270, 0.1945945946, 0.1216216216,
  0.0540540541, 0.0162162162
};

Texture2D		InputTexture : register(t0);
SamplerState	BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 dir;
	if (Direction == 0)
		dir = float2(1.f, 0.f);
	else
		dir = float2(0.f, 1.f);

	float3 texColor = InputTexture.Sample(BasicSampler, input.uv).rgb;
	float3 color = float3(0.f, 0.f, 0.f);
	float hstep = dir.x / Width;
	float vstep = dir.y / Height;

	for (int i = 0; i < 5; ++i)
	{
		float3 lColor = InputTexture.Sample(BasicSampler, input.uv + float2(hstep * offset[i], vstep * offset[i])).rgb * weight[i];
		float3 rColor = InputTexture.Sample(BasicSampler, input.uv - float2(hstep * offset[i], vstep * offset[i])).rgb * weight[i];
		color = color + (lColor + rColor);
	}

	texColor = color;

	return float4(texColor, 1.f);
}