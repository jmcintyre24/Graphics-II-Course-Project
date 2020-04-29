// Constant Buffer Variables
Texture2D txDiffuse : register(t0); // t for shader resource view.
Texture2D nrmMap : register(t1); // t for shader resource view.
SamplerState samLinear : register(s0); // s for samplers
TextureCube skybox : register(t2);

cbuffer ConstantBuffer : register(b0) // b for constant buffers
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir[2];
    float4 vLightColor[2];
    float4 vOutputColor;
    float time;
}

cbuffer UniqueBuffer : register(b1) // Definitly unncessary use here.
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
    float2 Tex : TEXCOORD0;
};

struct SKYBOX_VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tex : TEXCOORD2;
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

PS_INPUT VSWave(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos.y += 0.5f * sin(1 * output.Pos.x + time);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.worldPos = mul(input.Pos, World);
    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Tang = mul(input.Pos, World);
    output.Tex = input.Tex;
    return output;
}

SKYBOX_VS_INPUT SKYBOX_VS(SKYBOX_VS_INPUT input)
{
    SKYBOX_VS_INPUT output = (SKYBOX_VS_INPUT) 0;
    output.Pos = mul(float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f), World).xyww;
    
    output.Tex = input.Pos;
    return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shaders
//--------------------------------------------------------------------------------------
#define triCountOut 12
[maxvertexcount(triCountOut)]
void GS(triangle PS_INPUT input[3], inout TriangleStream<PS_INPUT> output)
{
    PS_INPUT simple[triCountOut];
    simple[0] = input[0];
    simple[1] = input[1];
    simple[2] = input[2];
    
    // Set everything other than the 'base' to the 'base'
    for (int i = 3; i < triCountOut; i += 3)
    {
        simple[i] = simple[0];
        simple[i].Pos = simple[0].worldPos; // Set the position to the world so we can make it stay stationary.
        simple[i + 1] = simple[1];
        simple[i + 1].Pos = simple[1].worldPos;
        simple[i + 2] = simple[2];
        simple[i + 2].Pos = simple[2].worldPos;
    }

    // First Little rock.
    for (int x = 3; x < 6; x++)
    {
        // Scale
        simple[x].Pos.x *= 0.25f;
        simple[x].Pos.y *= 0.25f;
        simple[x].Pos.z *= 0.25f;
        // Movement
        simple[x].Pos.x += 0.85f;
        simple[x].Pos.y -= 0.2f;
        simple[x].Pos.z += 0.5f;
    }
    
    // Second Little rock.
    for (int x = 6; x < 9; x++)
    {
        // Scale
        simple[x].Pos.x *= 0.25f;
        simple[x].Pos.y *= 0.25f;
        simple[x].Pos.z *= 0.25f;
        // Movement
        simple[x].Pos.x -= 0.75f;
        simple[x].Pos.y -= 0.25;
        simple[x].Pos.z -= 0.5f;
    }
    
    // Third Little rock.
    for (int x = 9; x < 12; x++)
    {
        // Scale
        simple[x].Pos.x *= 0.0625f;
        simple[x].Pos.y *= 0.0625f;
        simple[x].Pos.z *= 0.0625f;
        // Movement
        simple[x].Pos.x += 0.05f;
        simple[x].Pos.y += 0.27f;
        simple[x].Pos.z += 0.05f;
    }
    
    // Loop through and append everything to the stream.
    for (int z = 0; z < triCountOut; z++)
    {
        if(z >= 3) // Render the mesh 'normally' since we're using the mesh itself as the base, then restart the output strip and redo the points here.
        {
            if (fmod(z, 3) == 0 || simple[z].Pos.y < -0.1f) // Every three vertices, cut the output.
            {
                output.RestartStrip();
            }
            simple[z].Pos = mul(simple[z].Pos, View);
            simple[z].Pos = mul(simple[z].Pos, Projection);  
        }
        output.Append(simple[z]);
    }
}

[maxvertexcount(2)]
void GSWave(line PS_INPUT input[2], inout LineStream<PS_INPUT> output)
{
    // Get the base values in world space.
    input[0].Pos = input[0].worldPos;
    input[1].Pos = input[1].worldPos;
    
    // Modify them
    input[0].Pos.y += 0.5f * sin(1 * input[0].Pos.x + 1 * time);
    input[1].Pos.y += 0.5f * sin(1 * input[1].Pos.x + 1 * time);
    
    // Loop through and append everything to the stream.
    for (int i = 0; i < 2; i++)
    {
        // Output them
        input[i].Pos = mul(input[i].Pos, View);
        input[i].Pos = mul(input[i].Pos, Projection);
        output.Append(input[i]);
    }
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    // Have this value up to 0.075f for some ambient light.
    float4 finalColor = 0.075f;
        
    // Normal Map
    if (abs(input.Norm[0]) > 0)
    {
        float4 normMap = nrmMap.Sample(samLinear, input.Tex);
        
        normMap = (2.0f * normMap) - 1.0f;
        
        input.Tang = normalize(input.Tang - dot(input.Tang, input.Norm) * input.Norm);
        
        float3 tan = cross(input.Norm, input.Tang);
        
        float3x3 texSpace = float3x3(input.Tang, tan, input.Norm);
        
        input.Norm = normalize(mul(normMap, texSpace));
    }
    
    
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
    float4 finalColor = vOutputColor;
    float4 refColor = skybox.Sample(samLinear, input.Tang);
    return finalColor * refColor;
}

float4 PSNoLights(PS_INPUT input) : SV_Target
{    
    //float2 UV = { 0.5076, .5276 };
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    return color;
}

// Create a pulsation on the color w/ color change on position.
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

// SKYBOX PS
float4 SKYBOX_PS(SKYBOX_VS_INPUT input) : SV_Target
{    
    //finalColor *= txDiffuse.Sample(samLinear, input.Tex);
    
    //return finalColor;
    return vOutputColor * skybox.Sample(samLinear, input.Tex);
    //return vOutputColor;
}