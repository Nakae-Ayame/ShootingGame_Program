Texture2D gSceneTex : register(t0); //今フレームのシーンテクスチャ
Texture2D gPrevSceneTex : register(t1); //前フレームのシーンテクスチャ
SamplerState gSampler : register(s0); // サンプラーステート

//ポストエフェクト共通定数バッファ
cbuffer PostProcessCB : register(b0)
{
    float motionBlurAmount;
    float motionBlurStretch;
    float2 motionBlurCenter;

    float motionBlurStart01;
    float motionBlurEnd01;
    float2 pad0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

static const int SAMPLE_COUNT = 8;

//ピクセルシェーダーメイン関数
float4 PSMain(VS_OUT input) : SV_TARGET
{
    float2 uv = input.uv;
    
    if (motionBlurAmount <= 0.001f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }
    
    //float2 center = float2(0.5f, 0.5f);
    float2 center = motionBlurCenter;
    float dist = length(uv - center);
    
    float dist01 = dist / 0.70710678f;
    
    float denom = max(motionBlurEnd01 - motionBlurStart01, 1e-5f);
    float mask = saturate((dist01 - motionBlurStart01) / denom);
    mask = mask * mask * (3.0f - 2.0f * mask);

    float effectiveBlur = motionBlurAmount * mask;
    if (effectiveBlur <= 0.001f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }
    
    float2 dir = uv - center;
    float len = length(dir);
    if (len < 1e-4f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }
    
    dir /= len;

    float2 offsetBase = dir * motionBlurStretch * effectiveBlur;
    
    float3 col = 0.0f;
    float aSum = 0.0f;
    
    if (effectiveBlur <= 0.001f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }

    [unroll]
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float t = (i / (SAMPLE_COUNT - 1.0f));
        float2 offset = offsetBase * t;

        float4 s = gSceneTex.Sample(gSampler, uv + offset);
        col += s.rgb;
        aSum += s.a;
    }

    col /= SAMPLE_COUNT;
    float a = aSum / SAMPLE_COUNT;
    return float4(col, a);
    //return float4(motionBlurStart01, motionBlurEnd01, 0, 1);
}