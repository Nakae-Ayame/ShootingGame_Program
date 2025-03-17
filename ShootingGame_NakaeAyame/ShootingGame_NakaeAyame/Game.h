#pragma once
#include <iostream>

#include "Camera.h"
#include "Input.h"


#include "TitleScene.h"

enum SceneName
{
	TEST,
	TITLE,
	STAGE1,
	RESULT
};

class Game
{
private:

	static Game* m_Instance; // ゲームインスタンス

	Scene* m_Scene; // シーン

	std::vector<std::unique_ptr<Object>> m_Objects; // オブジェクト
	std::unique_ptr<Input> m_Input;  // 入力処理
	std::unique_ptr<Camera> m_Camera; // カメラ

public:
	//------------
	//関数
	//------------
	Game(); // コンストラクタ
	~Game(); // デストラクタ

	static void Init(); // 初期化
	static void Update(); // 更新
	static void Draw(); // 描画
	static void Uninit(); // 終了処理

	static Game* GetInstance();

	void ChangeScene(SceneName sName); // シーンを変更
	Camera& GetCamera(); // カメラ取得
	//void DeleteObject(Object* pt); // オブジェクトを削除する
	void DeleteAllObject(); // オブジェクトをすべて削除する


	// オブジェクトを追加する(※テンプレート関数なのでここに直接記述)
	/*template<class T> T* AddObject()
	{
		T* pt = new T(m_Camera.get());
		m_Instance->m_Objects.emplace_back(pt);
		pt->Init(); // 初期化
		return pt;
	}*/

	// オブジェクトを取得する(※テンプレート関数なのでここに直接記述)
	/*template<class T> std::vector<T*> GetObjects()
	{
		std::vector<T*> res;
		for (auto& o : m_Instance->m_Objects) {
			// dynamic_castで型をチェック
			if (T* derivedObj = dynamic_cast<T*>(o.get())) {
				res.emplace_back(derivedObj);
			}
		}
		return res;
	}*/

	//------------
	//関数
	//------------

};

