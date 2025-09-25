#include "TransitionManager.h"
#include "TransitionRenderer.h"
#include "SceneManager.h"
#include "Renderer.h"         // DrawFullScreenQuad 等（下で使う）
#include <cassert>
#include <iostream>
//--------------------------------------------------------
//                      初期化関数
//--------------------------------------------------------
void TransitionManager::Init()
{
    TransitionRenderer::Init();
}

void TransitionManager::Uninit()
{
    TransitionRenderer::Shutdown();
}

void TransitionManager::Start(const std::string& nextScene, float duration, TransitionType type, std::function<void()> preload)
{
    if (duration <= 0.0f) duration = 1.0f;
    m_IsActive = true;
    m_Duration = duration;
    m_Timer = 0.0f;
    m_NextScene = nextScene;
    m_Type = type;
    m_Preload = preload;
    m_HasSwitched = false;
}

//--------------------------------------------------------
//                      更新関数
//--------------------------------------------------------
void TransitionManager::Update(float deltaTime)
{
    if (!m_IsActive) return;

    std::cout << "[TR] Update: active=" << m_IsActive
        << " timer=" << m_Timer << "/" << m_Duration
        /*<< " hasLoaded=" << m_HasLoaded*/ << std::endl;

    m_Timer += deltaTime;
    float half = m_Duration * 0.5f;

    // 前半 (フェードアウト) が終わった段階でプリロード関数を呼び、
    // その後即座にシーンを切替える（単純同期版）
    if (!m_HasSwitched && m_Timer >= half)
    {
        if (m_Preload) {
            // シンプル実装: 同期的に実行
            m_Preload();
        }
        // シーン切替（同期的）
        SceneManager::SetCurrentScene(m_NextScene);
        m_HasSwitched = true;
    }

    if (m_Timer >= m_Duration)
    {
        m_IsActive = false;
        m_Timer = 0.0f;
        m_Preload = nullptr;
    }
}


//--------------------------------------------------------
//                     描画関数
//--------------------------------------------------------
void TransitionManager::Draw(float /*deltaTime*/)
{
    if (!m_IsActive) return;

    float progress = m_Timer / m_Duration;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    if (m_Type == TransitionType::FADE)
    {
        TransitionRenderer::DrawFade(progress);
    }
    else if (m_Type == TransitionType::IRIS)
    {
        TransitionRenderer::DrawIris(progress);
    }
}

bool TransitionManager::IsActive()
{
    return m_IsActive;
}


