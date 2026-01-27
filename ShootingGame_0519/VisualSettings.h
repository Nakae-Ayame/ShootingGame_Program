#pragma once

struct PostProcessSettings
{
	float motionBlurAmount = 0.0f;  //モーションブラーの強さ
	DirectX::XMFLOAT2 motionBlurDir = { 0.0f, 0.0f };	//モーションブラーの方向（画面上の2Dベクトル）	
	float motionBlurStretch;
	float bloomAmount;       //ブルームの強さ
	float vignetteAmount = 0.0f;    //ビネットの強さ
	float padding[2];
};

static_assert(sizeof(PostProcessSettings) == 32, "PostProcessCB size mismatch");
