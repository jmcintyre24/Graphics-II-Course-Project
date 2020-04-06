// Constant Buffer Variables
Texture2D txDiffuse : register(t0); // t for shader resource view.
SamplerState samLinear : register(s0); // s for samplers

cbuffer ConstantBuffer : register(b0) // b for constant buffers
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir[2];
    float4 vLightColor[2];
    float4 vOutputColor;
}

cbuffer UniqueBuffer : register(b1)
{
    float4 timePos;
}
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Tex = input.Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    float4 finalColor = 0;
    
    for (int i = 0; i < 2; i++)
    {
        finalColor += saturate(dot((float3) vLightDir[i], input.Norm) * vLightColor[i]);
    }
    finalColor *= txDiffuse.Sample(samLinear, input.Tex);
    finalColor.a = 1;
    return finalColor;
    
    //return txDiffuse.Sample(samLinear, input.Tex) * vOutputColor;
}

float4 PSSolid(PS_INPUT input) : SV_Target
{
    return vOutputColor;
}

// Color based off of time per frame and the directon of the light..
float4 PSUnique(PS_INPUT input) : SV_Target
{
    // Create the color using the X position, Y position, and time (up to 1)
    float4 color = { timePos[1], timePos[2], timePos[0], 1 };
    color = saturate(sin(color));
    color.a = 1;
    return color;
}