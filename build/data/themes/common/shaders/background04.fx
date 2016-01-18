#include "common.fx"
#include "simpleVS.fx"

#define SPEED 0.5f

#define LINESCOUNT 1

float ComputeCircle(float2 pos, float2 spherePos, float fRadius)
{
	float fDist = distance(spherePos, pos);
	return 1.0f-saturate(pow(fDist,4) * 10000.f/fRadius);
}
float4 mainImage( in float2 pos )
{
	float fTime = Time*SPEED;
	fTime = cos(fTime)+sin(fTime);
	
	float4 fColor = 0;
	
	fColor += ComputeCircle(pos, float2(0.5f,0.5f*fTime), 1.55f);
	fColor += ComputeCircle(pos, float2(0.5f,0.1f), 0.01f);
	
	return fColor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor = 0;
	
	return mainImage(input.Tex);
}