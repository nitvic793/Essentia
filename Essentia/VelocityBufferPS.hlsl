struct PixelInput
{
	float4 Position		: SV_POSITION;
	float4 PrevPosition	: POSITION;
};


//TODO: Output velocity buffer
// Also do intermediate rendering in HDR.
float4 main(PixelInput input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}