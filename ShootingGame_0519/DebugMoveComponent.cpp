#include "DebugMoveComponent.h"
#include "GameObject.h"

void DebugMoveComponent::Initialize()
{
    // 特に何もしない
}

void DebugMoveComponent::Update(float dt)
{
    // オーナーがいないなら何もしない
    GameObject* owner = GetOwner();
    if (!owner || dt <= 0.0f)
    {
        return;
    }

    Vector3 pos = owner->GetPosition();
    Vector3 move = Vector3::Zero;

    // --- 入力(WASD) ---
    // ※ Input クラスの実装に合わせて IsKeyDown / IsKeyPressed を必要に応じて変えてください
    //   （例えば DirectInput なら DIK_W / DIK_A... を使う形でも OK です）

    if (Input::IsKeyDown('W')) // 前
    {
        // ここでは「ワールドの -Z 方向」を前とする
        move.z -= 1.0f;
    }
    if (Input::IsKeyDown('S')) // 後ろ
    {
        move.z += 1.0f;
    }
    if (Input::IsKeyDown('A')) // 左
    {
        move.x -= 1.0f;
    }
    if (Input::IsKeyDown('D')) // 右
    {
        move.x += 1.0f;
    }

    // 入力がないなら何もしない
    if (move.LengthSquared() <= 1e-6f)
    {
        return;
    }

    // 斜め移動が速くなりすぎないように正規化
    move.Normalize();

    // 速度とデルタタイムを掛けて移動
    pos += move * m_moveSpeed * dt;

    owner->SetPosition(pos);
}

void DebugMoveComponent::Uninit()
{
    // 特に何もしない
}
