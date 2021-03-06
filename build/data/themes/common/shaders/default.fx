#include "common.fx"
#include "simpleVS.fx"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor = texture0.Sample(samplerTexture0, input.Tex);
	return fColor;
}