//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Resources
//--------------------------------------------------------------------------------------
Texture2D		texture0			: register(t0);
SamplerState	samplerTexture0		: register(s0)
{
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
	AddressU = Border;
	AddressV = Border;
	BorderColor = 0;
};



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
	float4 UserData;
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
