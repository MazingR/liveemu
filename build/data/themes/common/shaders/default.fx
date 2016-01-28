#include "common.fx"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	//input.Pos.w = 0;
	output.Pos = mul(input.Pos, World);
	//output.Pos.w = 1.0f;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Proj);
	
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor = texture0.Sample(samplerTexture0, input.Tex);
	//float fTx = (input.Tex.x - 0.5f) / 0.5f;
	//float fTy = (input.Tex.	y - 0.5f) / 0.5f;
	//float fPow = 8;

	// clip(fColor.a < 0.5f ? -1 : 1);
	//fColor *= (1 - pow(fTx, fPow)) * (1 - pow(fTy, fPow));
	// fColor.r = sin(Time);
	
	// float4 fColor = float4(1,0,0,1);
	return float4(0, fColor.r, 0,1);
	//return 1;
}