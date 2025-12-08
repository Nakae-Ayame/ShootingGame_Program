// MotionBlurPS.hlsl

Texture2D gSceneTex : register(t0); //今フレームのシーンテクスチャ
Texture2D gPrevSceneTex : register(t1); //前フレームのシーンテクスチャ
SamplerState gSampler : register(s0); // サンプラーステート

//ポストエフェクト共通定数バッファ
cbuffer PostProcessCB : register(b0) 
{
    float motionBlurAmount;     //0～1
    float2 motionBlurDir;       //今は未使用でもOK
    float motionBlurStretch;    //今は未使用でもOK

    float bloomAmount;    //今は未使用
    float vignetteAmount; //今は未使用
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

    //ブラー強度がほぼ 0 ならそのまま返す
    if (motionBlurAmount <= 0.001f)
    {
        return gSceneTex.Sample(gSampler, uv);
    }

    // とりあえず「横方向」に強いブラーをかけてみる (デバッグ用)
    // motionBlurStretch が 0.03 なら、画面幅の 3% 分くらい伸ばすイメージ
    float2 dir = float2(0.0, -1.0); // 横方向固定
    float2 offsetBase = dir * motionBlurStretch * motionBlurAmount;

    float3 col = 0;
    float wSum = 0;

    [unroll]
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float t = (i / (SAMPLE_COUNT - 1.0f)) - 0.5f; // -0.5 ～ +0.5
        float2 offset = offsetBase * t;
        float w = 1.0f; // 単純な等重み

        col += gSceneTex.Sample(gSampler, uv + offset).rgb * w;
        wSum += w;
    }

    col /= wSum;

    return float4(col, 1.0f);
}
