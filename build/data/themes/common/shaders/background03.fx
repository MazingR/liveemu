
// Source : https://www.shadertoy.com/user/iq

#include "common.fx"
#include "simpleVS.fx"

#define SPEED 0.01f
#define CIRCLES_COUNT 15
#define SATURATION 0.5f

void mainImage( out float4 fragColor, in float2 fragCoord )
{
	float fTime = Time*SPEED;
	float fD = (sin(fTime)+1.0f)*0.5f;
	
	float2 uv = -1.0 + 2.0*fragCoord.xy;
	uv.x *=  Resolution.x / Resolution.y;

	// background	
	float3 color = 0.8 + 0.2*uv.y;

	// bubbles	
	for( int i=0; i<CIRCLES_COUNT; i++ )
	{
		// bubble seeds
		float pha = sin(float(i)*546.13+1.0)*0.5 + 0.5;
		float siz = pow( sin(float(i)*651.74+5.0)*0.5 + 0.5, 4.0 );
		float pox = sin(float(i)*321.55+4.1);

		// buble size, position and color
		float rad = 0.1 + 0.5*siz;
		float2  pos = float2( pox, -1.0-rad + (2.0+2.0*rad)*fmod(pha+SPEED*Time*(0.2+0.8*siz),1.0));
		pos.y*=-1.f;
		float dis = length( uv - pos );
		float3  col = lerp( float3(fD,0.3,0.0), float3(1.0f-fD,0.4,0.8), 0.5+0.5*sin(float(i)*1.2+1.9));
		// col+= 8.0*smoothstep( rad*0.95, rad, dis );
		
		// render
		float f = length(uv-pos)/rad;
		f = sqrt(clamp(1.0-f*f,0.0,1.0))*SATURATION;
		color -= col.zyx *(1.0-smoothstep( rad*0.95, rad, dis )) * f;
	}

	// vigneting	
	color *= sqrt(1.5-0.5*length(uv));

	fragColor = float4(color,1.0);
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