Texture2D    texDiffuse : register(t0);
SamplerState sampLinear : register(s0);

struct PSInput
{
    float4 posH     : SV_POSITION;
    float4 col      : COLOR;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(sampLinear, float2(input.texcoord.x, 1.0f - input.texcoord.y)); // V を反転して試す
    return float4(texColor.rgb, 1.0f); // alpha を強制1.0（不透明）
}

/*    // 変更後（赤色で塗りつぶす）
    //return float4(1.0f, 0.0f, 0.0f, 1.0f); // 赤色
    float4 baseColor = input.col;
    float4 texColor = texDiffuse.Sample(sampLinear, input.texcoord);
    
    return baseColor * texColor;*/