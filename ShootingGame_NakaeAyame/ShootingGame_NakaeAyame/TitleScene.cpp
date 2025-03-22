#include "TitleScene.h"
#include "Game.h"
#include "Texture2D.h"

// コンストラクタ
TitleScene::TitleScene()
{
	Init();
}

// デストラクタ
TitleScene::~TitleScene()
{
	Uninit();
}

// 初期化
void TitleScene::Init()
{
	// 背景画像オブジェクトを作成 
	Texture2D* pt1 = Game::GetInstance()->AddObject<Texture2D>();
	pt1->SetTexture("assets/texture/background1.png"); // 画像を指定 
	pt1->SetPosition(0.0f, 0.0f, 200.0f); // 位置を指定 
	pt1->SetRotation(0.0f, 0.0f, 0.0f); // 角度を指定 
	pt1->SetScale(128.0f, 72.0f, 0.0f); // 大きさを指定 
	m_MySceneObjects.emplace_back(pt1);

	//sound.Init();
	//sound.Play(SOUND_LABEL_BGM001);
}

// 更新
void TitleScene::Update()
{
	// エンターキーを押してステージ1へ
	if (Input::GetKeyTrigger(VK_RETURN))
	{
		sound.Stop(SOUND_LABEL_BGM001);
		Game::GetInstance()->ChangeScene(STAGE1);
	}
}

// 終了処理
void TitleScene::Uninit()
{
	// このシーンのオブジェクトを削除する
	for (auto& o : m_MySceneObjects) {
		Game::GetInstance()->DeleteObject(o);
	}

}
