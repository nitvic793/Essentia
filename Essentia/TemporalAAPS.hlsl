
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer TemporalAAParams : register(b0)
{
	float2 ScreenSize;
}

Texture2D<float4>		InputTexture		: register(t0);
Texture2D<float4>		PrevFrameTexture	: register(t1);
Texture2D<float4>		VelocityTexture		: register(t2);
SamplerState			BasicSampler		: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 result = float3(0.f,0.f,0.f);
	float2 texelSize = 1.f / ScreenSize;

	float3 neighbourhood[9];

	neighbourhood[0] = InputTexture.Sample(BasicSampler, input.uv + float2(-1, -1) * texelSize).xyz;
	neighbourhood[1] = InputTexture.Sample(BasicSampler, input.uv + float2(+0, -1) * texelSize).xyz;
	neighbourhood[2] = InputTexture.Sample(BasicSampler, input.uv + float2(+1, -1) * texelSize).xyz;
	neighbourhood[3] = InputTexture.Sample(BasicSampler, input.uv + float2(-1, +0) * texelSize).xyz;
	neighbourhood[4] = InputTexture.Sample(BasicSampler, input.uv + float2(+0, +0) * texelSize).xyz;
	neighbourhood[5] = InputTexture.Sample(BasicSampler, input.uv + float2(+1, +0) * texelSize).xyz;
	neighbourhood[6] = InputTexture.Sample(BasicSampler, input.uv + float2(-1, +1) * texelSize).xyz;
	neighbourhood[7] = InputTexture.Sample(BasicSampler, input.uv + float2(+0, +1) * texelSize).xyz;
	neighbourhood[8] = InputTexture.Sample(BasicSampler, input.uv + float2(+1, +1) * texelSize).xyz;

	float3 nmin = neighbourhood[0];
	float3 nmax = neighbourhood[0];
	for (int i = 1; i < 9; ++i) {
		nmin = min(nmin, neighbourhood[i]);
		nmax = max(nmax, neighbourhood[i]);
	}

	float2 vel = VelocityTexture.Sample(BasicSampler, input.uv).xy;
	float2 histUv = input.uv + vel.xy;
	float3 histSample = clamp(PrevFrameTexture.Sample(BasicSampler, histUv).xyz, nmin, nmax);
	float blend = 0.05;

	bool a = (bool)(histUv > float2(1.0, 1.0));
	bool b = (bool)(histUv < float2(0.0, 0.0));
	blend = any(bool2(any(a), any(b))) ? 1.0 : blend;

	float3 curSample = neighbourhood[4];
	result = lerp(histSample, curSample, blend);

	return float4(result, 1.0f);
}