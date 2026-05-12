Texture2D texDiffuse : register(t0);
SamplerState sampLinear : register(s0);

cbuffer MaterialBuffer : register(b3)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shiness;
    bool TextureEnable;
    float2 Dummy;
};

struct PSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Diffuse : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(
        sampLinear,
        float2(input.TexCoord.x, 1.0f - input.TexCoord.y)
    );

    return float4(texColor.rgb, texColor.a * Diffuse.a);
}