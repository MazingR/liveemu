//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Resources
//--------------------------------------------------------------------------------------
Texture2D		texture0			: register(t0);
SamplerState	samplerTexture0		: register(s0);


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register(b0)
{
	matrix View;
	matrix Proj;
	float Time;
	float4 Resolution;
};

cbuffer cbPerObject : register(b1)
{
	matrix World;
};

//--------------------------------------------------------------------------------------
// Input/Outputs
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


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
	float4 fColor = float4(0,1,0,1);
	// float4 fColor = texture0.Sample(samplerTexture0, input.Tex);
	//float fTx = (input.Tex.x - 0.5f) / 0.5f;
	//float fTy = (input.Tex.	y - 0.5f) / 0.5f;
	//float fPow = 8;

	// clip(fColor.a < 0.5f ? -1 : 1);
	//fColor *= (1 - pow(fTx, fPow)) * (1 - pow(fTy, fPow));
	// fColor.r = sin(Time);
	// fColor.g = cos(Time);
	
	return fColor;
	//return 1;
}