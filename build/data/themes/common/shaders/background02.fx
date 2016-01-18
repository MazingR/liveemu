#include "common.fx"
#include "simpleVS.fx"

#define SPEED 0.1f
#define LIGHT_MIN 0.2f
#define LIGHT_MAX 0.8f
#define HORIZON 0.66f

float4 ComputeSpotLight(float2 pos, float2 fSpotPos, float2 fSpotSize, float fFallof)
{
	float2 fPos = float2(float2(pos+fSpotPos)/fSpotSize);
	float dist = 1.0f-distance(float2(0,0), fPos);
	return lerp(LIGHT_MIN, LIGHT_MAX, pow(dist, fFallof));
}

void mainImage( out float4 fragColor, in float2 pos )
{
	float fTimeOffset = abs(sin(Time*SPEED));
	
	// Upper half
	if (pos.y < HORIZON)
	{
		float fSpotPosX = fTimeOffset-1.0f;
		float fSpotPosY = cos(Time*SPEED)*0.2f -0.3f;
		
		float fLight = ComputeSpotLight(pos, float2(fSpotPosX, fSpotPosY), float2(1.0f, 0.6f), 1.0f);
		
		fragColor.rgb = fLight;
	}
	// Lower half
	else
	{
		float fSpotOffset = fTimeOffset*0.1f;
		float fLight = ComputeSpotLight(pos, float2(-0.5f+fSpotOffset, -1.0f), float2(0.9f, 0.35f), 1.0f);
		
		fragColor.rgb = fLight;
	}
	
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