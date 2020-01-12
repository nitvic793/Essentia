
cbuffer AOVSParams : register(b0)
{
    float4x4 InvProj;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 PosV     : POSITION;
    float2 UV       : TEXCOORD0;
};

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

VertexOut main(uint id : SV_VertexID)
{
  //  VertexOut output;

	 //Calculate the UV (0,0) to (2,2) via the ID
  //  output.UV = float2(
		//(id << 1) & 2, // id % 2 * 2
		//id & 2);

	 //Adjust the position based on the UV
  //  output.Position = float4(output.UV, 0, 1);
  //  output.Position.x = output.Position.x * 2 - 1;
  //  output.Position.y = output.Position.y * -2 + 1;

  //  float4 ph = mul(output.Position, InvProj);
  //  output.PosV = ph.xyz / ph.w;

  //  return output;
    VertexOut vout;

    vout.UV = gTexCoords[id];

    // Quad covering screen in NDC space.
    vout.Position = float4(2.0f * vout.UV.x - 1.0f, 1.0f - 2.0f * vout.UV.y, 0.0f, 1.0f);
 
    // Transform quad corners to view space near plane.
    float4 ph = mul(vout.Position, InvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}