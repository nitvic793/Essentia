
struct DirectionalLight
{
	float3 Direction;
	float3 Color;
};

struct PointLight
{
	float3	Position;
	float3	Color;
	float	Range;
};

float Attenuate(float3 position, float range, float3 worldPos)
{
	float dist = distance(position, worldPos);
	float att = saturate(1.0f - (dist * dist / (range * range)));
	return att * att;
}

float3 CalculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.Color * NdotL + float3(0.1, 0.1, 0.1);
}

float3 CalculatePointLight(float3 normal, float3 cameraPos, float3 worldPos, PointLight light)
{
	float3 toLight = normalize(light.Position - worldPos);
	float attenuation = Attenuate(light.Position, light.Range, worldPos);
	float lightAmount = dot(normal, toLight) * attenuation;
	lightAmount = saturate(lightAmount);
	return lightAmount * light.Color;
}