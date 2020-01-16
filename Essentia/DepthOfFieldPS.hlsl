
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer DepthOfFieldParams : register(b0)
{
	float NearZ;
	float FarZ;
	float FocusPlaneZ;
	float Scale;
};

float2 ProjectionConstants(float nearZ, float farZ)
{
	float2 projectionConstants;
	projectionConstants.x = farZ / (farZ - nearZ);
	projectionConstants.y = (-farZ * nearZ) / (farZ - nearZ);
	return projectionConstants;
}

float LinearZ(float depth)
{
	float2 projectionConstants = ProjectionConstants(NearZ, FarZ);
	float linearZ = projectionConstants.y / (depth - projectionConstants.x);
	return linearZ;
}

Texture2D			SceneTexture	: register(t0);
Texture2D			BlurTexture		: register(t1);
Texture2D<float>	DepthTexture	: register(t2);

SamplerState		BasicSampler	: register(s0);
SamplerState		LinearWrapSampler : register(s1);
SamplerState		PointClampSampler : register(s2);

float4 main(VertexToPixel input) : SV_TARGET
{
    float3 sharp = SceneTexture.Sample(LinearWrapSampler, input.uv).rgb;
    float3 blur = BlurTexture.Sample(LinearWrapSampler, input.uv).rgb;
    float depth = DepthTexture.Sample(PointClampSampler, input.uv).r;

	float linearZ = LinearZ(depth);
	float scale = 1.f - Scale;
	float radius = (FocusPlaneZ - linearZ) * (scale * scale);
	radius = clamp(radius * 2.0, -1.0, 1.0);
	float3 result = lerp(sharp.rgb, blur, abs(radius));

	return float4(result, 1.f);
}