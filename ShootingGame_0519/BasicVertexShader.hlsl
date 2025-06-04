cbuffer WorldBuffer : register(b0)
{
    matrix gWorld;
}
cbuffer ViewBuffer : register(b1)
{
    matrix gView;
}
cbuffer ProjBuffer : register(b2)
{
    matrix gProj;
}

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PS_Input
{
    float4 PosH : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

PS_Input VSMain(VS_Input input)
{
    PS_Input output;
    float4 worldPos = mul(float4(input.Position, 1.0f), gWorld);
    float4 viewPos = mul(worldPos, gView);
    output.PosH = mul(viewPos, gProj);
    output.Normal = input.Normal; // ä»à’ìIÇ…ïΩçsà⁄ìÆÇæÇØçlÇ¶ÇÈ
    output.TexCoord = input.TexCoord;
    return output;
}
