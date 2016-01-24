#include "common.fx"
#include "simpleVS.fx"

#define SPEED 0.02f

#define LINESCOUNT 12
#define SPHERES_COUNT 8
#define SIZE 20.01f
#define SIZE_DELTA 0.156f
#define SIZE_MAX 0.5f

#define INTENSITY 0.5045f
#define AMBIENT 0.01f
#define SATURATON 0.1f

float rand(float2 co)
{
	return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
}

float ComputeCircle(float2 pos, float2 spherePos, float fRadius)
{
	float fDist = distance(spherePos, pos);
	return 1.0f-saturate(pow(fDist,2) * 10.0f/fRadius);
}

float4 mainImage( in float2 pos )
{
	float fTime = Time*SPEED;
	float fD = (sin(fTime)+1.0f)*0.5f;
	
	float3 fColorA = float3(1.0f, 0.f, 0.f);
	float3 fColorB = float3(0.f, 0.0f, 1.0f);
	
	float3 fColorC = float3(0.f, 1.0f, 0.0f);
	float3 fColorD = float3(0.6f, 0.6f, 0.0f);
	
	float4 fColor = float4(lerp(fColorB, fColorD, sin(fTime)*0.5f + 0.5f), 1.f)*SATURATON + AMBIENT;
	
	for (int i=0; i<SPHERES_COUNT; ++i)
	{
		float fS = i;
		float fSx = sin(rand(float2(fS*fS, fS*fS*0.5f)))+0.05f;
		float fSy = sin(rand(float2(fS+fS, fS-fS))		)+0.05f;
		
		float4 s = float4(
		fSx,	// x
		fSy,	// y
		clamp(1.f/(fSx+fSy)*SIZE, 0.1f,SIZE_MAX),	// radius
		fSy );	// speed
		
		float fPosY = s.y +((fTime*s.w)%2) - fSy*2.f;
		s.y = 1.0f-fPosY;
		
		float fR = s.z + sin(fTime+fS)*SIZE_DELTA ;
		
		float4 fLightColor = float4(lerp(fColorA, fColorB, sin(fTime)*0.5f + 0.5f), 1.f)*SATURATON + 0.05f;
		fColor += ComputeCircle(pos, s.xy, fR)*INTENSITY*fLightColor;
	}
	
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