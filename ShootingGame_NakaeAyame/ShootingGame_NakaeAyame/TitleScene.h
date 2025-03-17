#pragma once
#include "Scene.h"

// TitleSceneクラス
class TitleScene : public Scene
{
private:
	void Init(); // 初期化
	void Uninit(); // 終了処理

public:
	TitleScene(); // コンストラクタ
	~TitleScene(); // デストラクタ

	void Update(); // 更新
};

