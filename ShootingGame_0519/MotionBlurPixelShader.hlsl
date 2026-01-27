// MotionBlurPS.hlsl

Texture2D gSceneTex : register(t0); //今フレームのシーンテクスチャ
Texture2D gPrevSceneTex : register(t1); //前フレームのシーンテクスチャ
SamplerState gSampler : register(s0); // サンプラーステート

//ポストエフェクト共通定数バッファ
cbuffer PostProcessCB : register(b0) 
{
    float motionBlurAmount;     //0～1
    float2 motionBlurDir;       //画面上の方向ベクトル
    float motionBlurStretch;    //

    float bloomAmount;    //
    float vignetteAmount; //
};

//フルスクリーンクアッドから渡される頂点データ
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//サンプル数
static const int SAMPLE_COUNT = 8;

//ピクセルシェーダーメイン関数
float4 PSMain(VS_OUT input) : SV_TARGET
{
    float2 uv = input.uv;
    
    if (motionBlurAmount <= 0.001f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }
    
    float2 center = float2(0.5f, 0.5f);
    float2 dir = uv - center;

    float len = length(dir);
    if (len < 1e-4f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }

    dir /= len;
    
    float effectiveBlur = motionBlurAmount;

    float2 offsetBase = dir * motionBlurStretch * effectiveBlur;
    
    float3 col = 0.0f;
    float wSum = 0.0f;

    [unroll]
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float t = (i / (SAMPLE_COUNT - 1.0f));
        float2 offset = offsetBase * t;

        float w = 1.0f;
        col += gSceneTex.Sample(gSampler, uv + offset).rgb * w;
        wSum += w;
    }

    col /= max(wSum, 1e-5f);
    
    float a = gSceneTex.Sample(gSampler, uv).a;
    return float4(col, a);
}

