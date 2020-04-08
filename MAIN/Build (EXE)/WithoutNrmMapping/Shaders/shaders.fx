// Constant Buffer Variables
Texture2D txDiffuse : register(t0); // t for shader resource view.
Texture2D nrmMap : register(t1); // t for shader resource view.
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
    float4 worldPos : POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
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
    output.worldPos = mul(input.Pos, World);
    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Tang = mul(input.Pos, World);
    output.Tex = input.Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    // Have this value up to 0.075f for some ambient light.
    float4 finalColor = 0.075f;
        
    // Normal Map
    //if (abs(input.Norm[0]) > 0)
    //{
    //    float4 normMap = nrmMap.Sample(samLinear, input.Tex);
        
    //    normMap = (2.0f * normMap) - 1.0f;
        
    //    input.Tang = normalize(input.Tang - dot(input.Tang, input.Norm) * input.Norm);
        
    //    float3 tan = cross(input.Norm, input.Tang);
        
    //    float3x3 texSpace = float3x3(input.Tang, tan, input.Norm);
        
    //    input.Norm = normalize(mul(normMap, texSpace));
    //}
    
    
    // Apply Lighting
    for (int i = 0; i < 2; i++)
    {
        // Directional Lighting
        if(i == 0)
        {
            finalColor += saturate(dot((float3) vLightDir[i], input.Norm) * vLightColor[i]);
        }
        // Point Lighting
        else if (i == 1)
        {
            float4 lightDir = normalize(vLightDir[i] - input.worldPos);
            float distance = length(lightDir);
            
            // Apply the point light to the color if within range.
            if (distance <= 1.5f)
            {
                finalColor += saturate(dot((float3) lightDir, input.Norm) * vLightColor[i]);
            }
        }
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

// Create a pulsation on the color
float4 PSUnique(PS_INPUT input) : SV_Target
{
    float4 color = 0;
    // Create the color using the X position, Z position, and time (timePos[0])

    color.r = saturate(cos(abs(input.worldPos[0]) + abs(input.worldPos[2])));
    color.r += saturate(cos(abs(input.worldPos[0])));
    color.g = saturate(cos(abs(input.worldPos[2]) + abs(input.worldPos[0])));
    color.g += saturate(cos(abs(input.worldPos[2])));
    color.b = saturate(sin(timePos[0]));
            
    color.a = 1;
    return color;
}