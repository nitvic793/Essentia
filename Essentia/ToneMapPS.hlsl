
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D<float4>		InputTexture		: register(t0);
SamplerState			BasicSampler		: register(s0);
SamplerState			LinearWrapSampler	: register(s1);

// linear white point
static const float W = 1.2;
static const float T2 = 7.5;

float FilmicReinhardCurve(float x)
{
    float q = (T2 * T2 + 1.0) * x * x;
    return q / (q + x + T2 * T2);
}

float3 FilmicReinhard(float3 x)
{
    float w = FilmicReinhardCurve(W);
    return float3(
        FilmicReinhardCurve(x.r),
        FilmicReinhardCurve(x.g),
        FilmicReinhardCurve(x.b)) / w;
}

float3 ToneMapFilmicALU(float3 color)
{
	color = max(0, color - 0.004f);
	color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

	// result has 1/2.2 baked in
	return pow(color, 2.2f);
}

float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float4 main(VertexToPixel input) : SV_TARGET
{
    float3 color = InputTexture.Sample(LinearWrapSampler, input.uv).rgb;
    color = ToneMapFilmicALU(color);
	return float4(saturate(color), 1.0f);
}