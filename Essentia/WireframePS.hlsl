
struct GSOutput
{
    float4 Position : SV_POSITION;
    float3 Bary : TEXCOORD0;
};

float4 main(GSOutput input) : SV_Target
{
    const float WireframeVal = 0.1f;
    const float4 Color = float4(1.f, 0.f, 0.f, 1.f);
    if (!any(bool3(input.Bary.x < WireframeVal, input.Bary.y < WireframeVal, input.Bary.z < WireframeVal)))
        discard;

    return Color;
}