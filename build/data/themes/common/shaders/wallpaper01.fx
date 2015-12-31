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

float speed = 1.4f;
float widthFactor = 2.0f;

float3 calcSine(float2 uv, 
              float frequency, float amplitude, float shift, float offset,
              float3 color, float width, float exponent, bool dir)
{
    float angle = Time * speed * frequency + (shift + uv.x) * 0.75f;
    
    float y = sin(angle) * amplitude + offset;
	// y*=0.1f;
    float diffY = y-uv.y;
    
    float dsqr = distance(y,uv.y);
    float scale = 1.0f;
    
    if(dir && diffY > 0.0f)
    {
		// return float3(dsqr,0,0);
        // dsqr = dsqr * 4.0f;
    }
    else if(!dir && diffY < 0.0f)
    {
		// return float3(0,dsqr,0);
        // dsqr = dsqr * 4.0f;
    }
    
	if (dsqr>=(width * widthFactor))
		return float3(dsqr,dsqr,dsqr);
	else
		return float3(dsqr,dsqr,dsqr);
	
	float fStep = smoothstep(0.0f, width * widthFactor, dsqr);
    scale = pow(1.0f-fStep, exponent);
    
    return min(color * scale, color);
}

float4 mainImage( float2 fragCoord )
{
    float2 uv = fragCoord;
	uv.y = 1-uv.y;
	
    float3 color = 0.0f;
	
    float t1 = (sin(Time/20.0f) / 3.14f) + 0.2f;
	float t2 = (sin(Time/10.0f) / 3.14f) + 0.2f;
	
    float3 baseColor = float3(0.5f, 0.5f, 0.5f);
	
    color += calcSine(uv, 0.20f, 0.2f, 0.0f, 0.5f,  baseColor, 0.1f, 15.0f,false);
    color += calcSine(uv, 0.40f, 0.15f, 0.2f, 0.5f, baseColor, 0.1f, 17.0f,false);
    color += calcSine(uv, 0.60f, 0.15f, 0.0f, 0.5f, baseColor, 0.05f, 23.0f,false);
    
    color += calcSine(uv, 0.26f, 0.07f, 0.0f, 0.3f, baseColor, 0.1f, 17.0f,true);
    color += calcSine(uv, 0.46f, 0.07f, 0.0f, 0.3f, baseColor, 0.05f, 23.0f,true);
    color += calcSine(uv, 0.58f, 0.05f, 0.0f, 0.3f, baseColor, 0.2f, 15.0f,true);

    color.x += t1 * (1.0f-uv.y);
	color.y += t2 * (1.0f-uv.y);
    
    return float4(color,1.0f);
}


/*
// starDust - shadertoy intro
// Created by Dmitry Andreev - and'2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#define SPEED           (0.15f)

#define MIN_LINES       (22.0f)
#define MAX_LINES       (28)

#define MIN_LINES_Y     (2.0f)
#define MAX_LINES_Y     (2)

#define SATURATION		(0.8f)
#define WARMUP_TIME     (0.0f)

// Shadertoy's sound is a bit out of sync every time you run it :(
#define SOUND_OFFSET    (-0.0)

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float isectPlane(float3 n, float d, float3 org, float3 dir)
{
    float t = -(dot(org, n) + d) / dot(dir, n);

    return t;
}

float drawLogo(in float2 fragCoord)
{
    float res = max(iResolution.x, iResolution.y);
    float2  pos = float2(floor((fragCoord.xy / res) * 128.0f));

    float val = 0.0f;

    // AND'14 bitmap
    if (pos.y == 2.0f) val = 4873761.5f;
    if (pos.y == 3.0f) val = 8049199.5f;
    if (pos.y == 4.0f) val = 2839721.5f;
    if (pos.y == 5.0f) val = 1726633.5f;
    if (pos.x >125.0f) val = 0.0f;

    float bit = floor(val * exp2(pos.x - 125.0f));	

    return bit != floor(bit / 2.0f) * 2.0f ? 1.0f : 0.0f;
}

float3 drawEffect(float2 coord, float time)
{    
    float3 clr = 0.0f;
    float mtime = sin(time)*64.0f;
    const float far_dist = 10000.0f;

    float2 uv = coord.xy / iResolution.xy;

    float3 org = 0.0f;
    float3 dir = float3(uv.xy * 2.0f - 1.0f, 1.0f);

    // Animate tilt
    float ang = sin(time * 0.2f) * 0.2f;
    float3 odir = dir;
    dir.x = cos(ang) * odir.x + sin(ang) * odir.y;
    dir.y = sin(ang) * odir.x - cos(ang) * odir.y;

    // Animate FOV and aspect ratio
    dir.x *= 1.5f + 0.5f * sin(time * 0.125f);
    dir.y *= 1.5f + 0.5f * cos(time * 0.25f + 0.5f);

    // Animate view direction
    dir.x += 0.25f * sin(time * 0.3f);
    dir.y += 0.25f * sin(time * 0.7f);

    // Cycle between long blurry and short sharp particles
    float2 param = lerp(float2(60.0f, 0.8f), float2(800.0f, 3.0f),
        pow(0.5f + 0.5f * sin(time * 0.2f), 2.0f));

    float lt = frac(mtime / 4.0f) * 4.0f;
    float2 mutes = 0.0f;
    
    if (mtime >= 32.0f && mtime < 48.0f)
    {
        mutes = max(float2(0.0f, 0.0f), 1.0f - 4.0f * abs(lt - float2(3.25f, 3.50f)));
    }
    
    for (int k = 0; k < MAX_LINES_Y; k++)
    for (int i = 0; i < MAX_LINES; i++)
    {
        int iLinesX = int(max(mtime, MIN_LINES));
        int iLinesY = int(max(mtime, MIN_LINES_Y));
        
        if (iLinesX>i && iLinesY>k)
        {
            float3 pn = float3(k > 0 ? -1.0f : 1.0f, 0.0f, 0.0f);
            float t = isectPlane(pn, 100.0f + float(i) * 20.0f, org, dir);

            if (t <= 0.0f || t >= far_dist) continue;

            float3 p = org + dir * t;
            float3 vdir = normalize(-p);

            // Create particle lanes by quantizing position
            float3 pp = ceil(p / 100.0f) * 100.0f;

            // Pseudo-random variables
            float n = pp.y + float(i) + float(k) * 123.0f;
            float q = frac(sin(n * 123.456f) * 234.345f);
            float q2= frac(sin(n * 234.123f) * 345.234f);

            q = sin(p.z * 0.0003f + 1.0f * time * (0.25f + 0.75f * q2) + q * 12.0f);

            // Smooth particle edges out
            q = saturate(q * param.x - param.x + 1.0f) * param.y;
            q *= saturate(4.0f - 8.0f * abs(-50.0f + pp.y - p.y) / 100.0f);

            // Fade out based on distance
            q *= 1.0f - saturate(pow(t / far_dist, 5.0f));

            // Fade out based on view angle
            float fn = 1.0f - pow(1.0f - dot(vdir, pn), 2.0f);
            q *= 2.0f * smoothstep(0.0f, 1.0f, fn);

            // Cycle palettes
            const float3 orange = float3(1.0f, 0.7f, 0.4f);
            const float3 blue   = float3(0.4f, 0.7f, 1.0f);
            clr += q * lerp(orange, blue, 0.5f + 0.5f * sin(time * 0.15f + q2));
        }
    }

    //clr *= 0.2f;

    // Cycle gammas
    clr.r = pow(clr.r, 0.75f + 0.35f * sin(time * 0.5f));
    clr.b = pow(clr.b, 0.75f - 0.35f * sin(time * 0.5f));

    // Initial fade-in
    //clr *= pow(min(mtime / 4.0f, 1.0f), 2.0f);

    // Fade-out shortly after initial fade-in right before drums kick in
    if (mtime < 8.0f) clr *= 1.0f - saturate((mtime - 5.0f) / 3.0f);

    // Flash horizon in sync with snare drum
    if (mtime >= 15.0f)
    {
        float h = normalize(dir).x;
        clr *= 1.0f + 2.0f * pow(saturate(1.0f - abs(h)), 8.0f)
            * max(0.0f, frac(-mtime + 0.5f) * 4.0f - 3.0f);
    }

    // Vignette in linear space (looks better)
    clr *= clr;
    clr *= 1.4f;
    clr *= 1.0f - 1.5f * dot(uv - 0.5f, uv - 0.5f);
    clr = sqrt(max(float3(0.0f, 0.0f, 0.0f), clr));

    return clr;
}

float4 mainImage(float2 fragCoord )
{
    float time = max(0.0f, Time*100.0f - WARMUP_TIME);
    float3  clr = 0.0f;

	time = SOUND_OFFSET + time*SPEED;
    
    clr = drawEffect(fragCoord.xy, time);
    // clr = lerp(clr, float3(0.8f, 0.9f, 1.0f), 0.3f * drawLogo(fragCoord));

    return float4(clr*SATURATION, 0.0f);
}
*/
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 fColor = mainImage(input.Tex);
	//float fTx = (input.Tex.x - 0.5f) / 0.5f;
	//float fTy = (input.Tex.	y - 0.5f) / 0.5f;
	//float fPow = 8;

	// clip(fColor.a < 0.5f ? -1 : 1);
	//fColor *= (1 - pow(fTx, fPow)) * (1 - pow(fTy, fPow));

	return fColor;
	//return 1;
}
