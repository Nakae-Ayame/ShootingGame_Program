#pragma once

struct PostProcessSettings
{
    float motionBlurAmount;      
    float motionBlurStretch;     
    DirectX::XMFLOAT2 motionBlurCenter;

    float motionBlurStart01;     
    float motionBlurEnd01;       
    float padding[2];            
};
static_assert(sizeof(PostProcessSettings) == 32, "CB size mismatch");


///// <summary>
///// モーションブラー用
///// 定数バッファに送る用の構造体
///// </summary>
//struct PostProcessSettings
//{
//    float motionBlurAmount = 0.0f;
//    float motionBlurStretch = 0.0f;
//    float motionBlurStart01 = 0.0f;
//    float motionBlurEnd01 = 1.0f;
//
//    DirectX::XMFLOAT2 motionBlurCenter01 = { 0.5f, 0.5f };
//    DirectX::XMFLOAT2 motionBlurDir      = { 0.0f, 0.0f };
//
//    float bloomAmount = 0.0f;
//    float vignetteAmount = 0.0f;
//    float padding[2] = { 0.0f, 0.0f };
//};