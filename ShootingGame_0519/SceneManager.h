#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "NonCopyable.h"

class IScene;

/// <summary>
/// Scene遷移などを管理するクラス
/// (コピー禁止で生成)
/// </summary>
class SceneManager : NonCopyable
{
public:
	//--------------Scene登録・切り替え関連------------------
	static void RegisterScene(const std::string& name, std::unique_ptr<IScene> scene);
	static void SetCurrentScene(const std::string& name);
	static void SetChangeScene(const std::string& name);
	static std::string GetCurrentSceneName();

	//--------------更新・描画・初期化・終了関連------------------
	static void Update(float deltatime);
	static void Draw(float deltatime);
	static void DrawWorld(float deltatime);
	static void DrawUI(float deltatime);
	static void Init();
	static void Uninit();

private:
	//--------------Scene内部処理関連------------------
	static void ChangeSceneInternal(const std::string& name);
	static void UpdateWindowTitle(const std::string& title);

	//--------------Scene管理関連------------------
	static std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
	static std::string m_currentSceneName;
	static bool m_sceneChangedThisFrame;
};