Texture2D    texDiffuse : register(t0);
SamplerState sampLinear : register(s0);

struct PSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Diffuse  : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(sampLinear, float2(input.TexCoord.x, 1.0f - input.TexCoord.y)); // V ‚ğ”½“]‚µ‚Ä‚·
    return float4(texColor.rgb, 1.0f); // alpha ‚ğ‹­§1.0i•s“§–¾j
}
