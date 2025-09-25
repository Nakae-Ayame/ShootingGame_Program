#pragma once
#include <string>
#include <functional>

enum class TransitionType
{
    FADE,
    IRIS
};

class TransitionManager
{
public:
    static void Init(); // 必要なら呼ぶ（シェーダ初期化）
    static void Uninit();

    // nextScene: 切替先のシーン名（非同期ロードしない単純版では SetCurrentScene を呼ぶ）
    // duration: 秒
    // preload: 遷移中に実行したい（同期）処理。非同期実装は別途。
    static void Start(const std::string& nextScene,
        float duration,
        TransitionType type = TransitionType::FADE,
        std::function<void()> preload = nullptr);

    static void Update(float deltaTime);
    static void Draw(float deltaTime);
    static bool IsActive();

private:
    static inline bool m_IsActive = false;
    static inline float m_Duration = 1.0f;
    static inline float m_Timer = 0.0f;
    static inline std::string m_NextScene;
    static inline TransitionType m_Type = TransitionType::FADE;
    static inline std::function<void()> m_Preload = nullptr;
    static inline bool m_HasSwitched = false; // 既にシーンを切替えたか
};
