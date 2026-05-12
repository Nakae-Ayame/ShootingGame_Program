#include <chrono>
#include "Game.h"
#include "renderer.h"
#include "SceneManager.h"
#include "Application.h"
#include "Sound.h"
#include "TransitionManager.h"
#include "DebugUI.h"
#include "EffectManager.h"

void Game::GameInit()
{
    //Application::HideCursorAndClip(); 

    Renderer::Init();
    
    Sound::Init();

    TransitionManager::Init();

    EffectManager::Init();

    SceneManager::Init();

    DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());
}

void Game::GameUninit()
{
    DebugUI::DisposeUI();

    SceneManager::Uninit();

    EffectManager::Uninit();

    TransitionManager::Uninit();

    Sound::Uninit();

    Renderer::Uninit();
}

void Game::GameUpdate(float deltaTime)
{
    Sound::Update(deltaTime);

    SceneManager::Update(deltaTime);

	EffectManager::Update(deltaTime);

    TransitionManager::Update(deltaTime);
}

void Game::GameDraw(float deltaTime)
{    
    Renderer::Begin();

    SceneManager::DrawWorld(deltaTime);

    EffectManager::Draw3D(deltaTime);

    Renderer::ApplyMotionBlur();

    SceneManager::DrawUI(deltaTime);

    TransitionManager::Draw(deltaTime);

    Renderer::End();
}