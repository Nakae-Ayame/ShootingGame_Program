#pragma once
#include "Texture2D.h"

class Test2DImage :public Texture2D
{
public:
	Test2DImage(Camera* cam) : Texture2D(cam) //コンストラクタ
	{

	}
	~Test2DImage() //デストラクタ
	{

	}

	//void Init();
	void Update();
	//void Draw();
private:
	int m_animationCount = 0;		//animation切り替え用のcount
	const int m_coinSpriteU = 10;
};