#include "TitleBackGround.h"
#include "Input.h"
#include "Renderer.h"
#include "Application.h"
#include <Windows.h>

TitleBackGround::TitleBackGround(const std::wstring& texturePath, float size)
    : m_texturePath(texturePath), m_size(size)
{
    m_pos = Vector2(0.0f, 0.0f);
}

void TitleBackGround::Initialize()
{
    //panda_iruka_irowake_iruka.png
    // 自分のコンポーネントとして TextureComponent を作る
    m_texture = AddComponent<TextureComponent>();
    if (!m_texture)
    {
        OutputDebugStringA("Reticle: TextureComponent 作成失敗\n");
        return;
    }

    // テクスチャ読み込み（ワーキングディレクトリに依存）
    if (!m_texture->LoadTexture(m_texturePath))
    {
        OutputDebugStringA("Reticle: テクスチャ読み込み失敗\n");
    }
    m_texture->SetSize(50, 20);

    // 初期位置：ウィンドウ中央
    RECT rc{};
    GetClientRect(Application::GetWindow(), &rc);
    m_pos.x = static_cast<float>((rc.right - rc.left) / 2);
    m_pos.y = static_cast<float>((rc.bottom - rc.top) / 2);

    // TextureComponent は SetScreenPosition が左上基準なのでここで設定しておく
    float left = m_pos.x - m_size * 0.5f;
    float top = m_pos.y - m_size * 0.5f;
    m_texture->SetScreenPosition(left, top);
}

void TitleBackGround::Update(float dt)
{
    
}

void TitleBackGround::Draw(float alpha)
{
    if (!m_texture || !m_texture->GetSRV()) return;

    // Renderer::DrawReticle は中心座標の POINT を期待する実装を想定
    POINT center{ static_cast<LONG>(m_pos.x), static_cast<LONG>(m_pos.y) };
    Vector2 size(1280, 720);

    // DrawReticle 内で深度・ブレンドの切り替えを行い、
    // DrawTexture 側で SRV のアンバインドとシェーダ復帰を行うことを期待
    Renderer::DrawReticle(m_texture->GetSRV(), center, size);
}