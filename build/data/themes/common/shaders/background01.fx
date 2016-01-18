#include "common.fx"
#include "simpleVS.fx"

// Source : https://www.shadertoy.com/user/anastadunbar 

#define SPEED 0.2f
#define CIRCLES_COUNT 6

float3 HueShift (in float3 Color, in float Shift)
{
	float3 fA = 0.55735;
	
	float3 P = fA*dot(fA,Color);
	float3 U = Color-P;
	float3 V = cross(fA,U);	
	Color = U*cos(Shift*6.2832) + V*sin(Shift*6.2832) + P;
	return float3(Color);
}

float2 dir_dist(float dir, float dist) 
{
	return float2(cos(dir)*dist,sin(dir)*dist);
}

float3 animation(float2 uv, float time) {
	uv = uv-0.5;
	float circles = 0.;
	float pi = 3.14159265358979323;
	for (float k = 0.; k < CIRCLES_COUNT; k++) {
		float i = (k/CIRCLES_COUNT)*pi;
		#define DIRECTION sin(time+i)
		#define DISTANCE sin(time-(i*1.34))
		circles += clamp(1.-(length(uv-dir_dist((DIRECTION),(DISTANCE)*0.2))*38.),0.,1.);
	}
	circles = clamp(circles,0.,1.);
	float3 fCircle = circles;
	float3 res = pow(fCircle,float3(2.,1.4,1.));
	return res;
}
void mainImage( out float4 fragColor, in float2 fragCoord )
{
	float2 uv = fragCoord.xy;// / Resolution.xy;
	float2 suv = float2(((uv.x-0.5)*(Resolution.x / Resolution.y))+0.5,uv.y);
	float time = Time*SPEED;
	
	float3 drawing = animation(suv,time);
	float i = 0;
	
	for (i = 0.; i < 30.; i++) {
		drawing += (animation(suv,time-(i*.011))/(i+1.))*0.3;
	}
	
	drawing = float3(pow(drawing,float3(3.,1.4,1.)));
	drawing = lerp(drawing,HueShift(drawing,0.5),0.3);
	
	float3 drawing3 = 0.0f;
	float3 fC = 7.0f;
	for (i = 0.; i < 40.; i++) {
		drawing3 += ((pow((animation(suv,time-(i*.018))),fC)*(1.-(i/40.))))/20.;
	}
	drawing3 = float3(pow(drawing3,float3(4.,1.8,1.)));
	drawing += clamp((1.-(((length(suv-0.5)-0.2)+0.2)*2.)),0.,1.)*float3(0.,0.03,0.1);
	
	fragColor = float4(drawing+drawing3,1.0);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor;
	mainImage(fColor, input.Tex);
	
	return fColor;
}