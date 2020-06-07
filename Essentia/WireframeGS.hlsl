
#define REMOVE_DIAG 1

struct GSInput
{
    float4 Position : SV_POSITION;
    float4 WorldPos : POSITION;
};

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};

struct GSOutput
{
	float4 Position : SV_POSITION;
    float3 Bary     : TEXCOORD0;
};

[maxvertexcount(3)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
    float3 param = float3(0., 0., 0.);
	#ifdef REMOVE_DIAG
    float EdgeA = length(input[0].WorldPos - input[1].WorldPos);
    float EdgeB = length(input[1].WorldPos - input[2].WorldPos);
    float EdgeC = length(input[2].WorldPos - input[0].WorldPos);

    if(EdgeA > EdgeB && EdgeA > EdgeC)
        param.y = 1.;
    else if (EdgeB > EdgeC && EdgeB > EdgeA)
        param.x = 1.;
    else
        param.z = 1.;
    #endif
    
    float3 baryOffset[3] =
    {
        float3(1.f, 0.f, 0.f),
        float3(0.f, 0.f, 1.f),
        float3(0.f, 1.f, 0.f)
    };
    
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
        element.Position = input[i].Position;
        element.Bary = baryOffset[i] + param;
		output.Append(element);
	}
}