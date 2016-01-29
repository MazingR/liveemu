#include "common.fx"
#include "simpleVS.fx"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float2 vCoord = input.Tex;
	vCoord *= UserData.zw;
	vCoord += UserData.xy;
	float4 fColor = texture0.Sample(samplerTexture0, vCoord);
	return fColor.r;
}