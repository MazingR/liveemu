#include "common.fx"
#include "simpleVS.fx"

#define SPEED 1.5f

#define LINESCOUNT 2

float ComputeLine(float2 pos, float fFreq, float fXOffset, float fAmp, float fYOffset)
{
	return sin((pos.x*fFreq)+fXOffset)*fAmp + fYOffset;
}
float ComputeLineIntensity(float2 pos, float fLine, float fSize)
{
	float fC = (pos.y-fLine)*(100.0f*fSize+150.0f);
	return lerp(0, 1, 1.0f-pow(fC, 8));
}
void mainImage( out float4 fragColor, in float2 pos )
{
	float fTime = Time*SPEED;
	// float fTimeOffset = (sin(fTime)+cos(fTime)+1.5f)*0.33f;
	float fTimeOffset = sin(fTime)+cos(fTime);
	
	float fX = pos.x;
	float fXVariant = cos(fX+fTimeOffset*0.5f);
	float fFreq = 2.0f;
	float fAmp = 0.85f;
	float fYOffset = 0.4f;
	float fXOffset = fTimeOffset;
	float fSize = fTimeOffset;
	
	float3 c = 0;
	float3 vLineColor = 0.5f;
	
	float fLine;
	float fFactor = 0.5f;
	
	// fLine = ComputeLine(pos, 2.0f, fTimeOffset, 0.15f, 0.4f);
	// c += ComputeLineIntensity(pos, fLine, fSize) * vLineColor * fFactor;
	
	// fLine = ComputeLine(pos, 2.0f, fTimeOffset+0.4f, 0.25f, 0.1f);
	// c += ComputeLineIntensity(pos, fLine, fSize) * float3(0.23,0.26,0.23) * fFactor;
	
	fragColor.rgb = (c * float3(0.9f, 0.9f, 1));// + float3(0.09f, 0.09f, 0.1);
	fragColor.a = 1.0f;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor = 0;
	mainImage(fColor, input.Tex);
	
	return fColor;
}