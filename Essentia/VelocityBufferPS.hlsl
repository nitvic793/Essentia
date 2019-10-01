struct PixelInput
{
	float4 Position		: SV_POSITION;
	float4 PrevPosition	: POSITION;
};


//TODO: Output velocity buffer
// Also do intermediate rendering in HDR.
float4 main(PixelInput input) : SV_TARGET
{
	float2 vecA = (input.Position.xy / input.Position.w) * 0.5f + 0.5;
	float2 vecB = (input.PrevPosition.xy / input.PrevPosition.w) * 0.5f + 0.5;
	float2 outVelocity = vecA - vecB;
	return float4(outVelocity, 0.f, 0.f);
}