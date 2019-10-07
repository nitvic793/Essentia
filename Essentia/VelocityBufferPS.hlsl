struct PixelInput
{
	float4	Position		: SV_POSITION;
	float4	CurrentPosition	: POSITION0;
	float4	PrevPosition	: POSITION1;
};

float4 main(PixelInput input) : SV_TARGET
{
	float2 vecA = (input.CurrentPosition.xy / input.CurrentPosition.w) * 0.5f + 0.5;
	float2 vecB = (input.PrevPosition.xy / input.PrevPosition.w) * 0.5f + 0.5;
	float2 outVelocity = vecA - vecB;
	return float4(outVelocity, 0.f, 1.f);
}