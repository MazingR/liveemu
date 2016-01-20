#include "common.fx"
#include "simpleVS.fx"

#define SPEED 0.15f

#define LINESCOUNT 1
#define SPHERES_COUNT 16
#define SIZE 20.f
#define SIZE_DELTA 0.01f

float rand(float2 co)
{
	return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
}

float ComputeCircle(float2 pos, float2 spherePos, float fRadius)
{
	float fDist = distance(spherePos, pos);
	return 1.0f-saturate(pow(fDist,2) * (1000.0f/SIZE)*fRadius);
}
float4 mainImage( in float2 pos )
{
	float fTime = Time*SPEED;
	float fD = (sin(fTime)+1.0f)*0.5f;
	float4 fColor 	= 0;
	
	for (int i=0; i<SPHERES_COUNT; ++i)
	{
		float fS = i;
		float fSx = sin(rand(float2(fS*fS, fS*fS*0.5f)))+0.05f;
		float fSy = sin(rand(float2(fS+fS, fS-fS))		)+0.05f;
		
		float4 s = float4(
		fSx,	// x
		fSy,	// y
		(fSx+fSy),	// radius
		fSy );	// speed
		
		float fPosY = s.y +((fTime*s.w)%2) - fSy*2.f;
		s.y = 1.0f-fPosY;
		
		float fRd = 5.f + SIZE_DELTA*fS;
		float fR = s.z + s.z*(sin(fTime+fS)*fRd+fRd);
		
		fColor += ComputeCircle(pos, s.xy, fR);
		// fColor *= float4(rand(s.xy), rand(s.yx), rand(s.yx), 1.f) + 0.3f;
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