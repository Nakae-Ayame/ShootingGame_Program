#include "MoveComponent.h"
#include "GameObject.h"
#include <iostream>
//#include <DirectXMath.h>

using namespace DirectX::SimpleMath;

constexpr float XM_PI = 3.14159265f;
constexpr float XM_2PI = 6.28318530f;

void MoveComponent::Initialize()
{


}

void MoveComponent::Update()
{
    //カメラがセットされていない場合は処理を終了する
    if (!m_camera) return;

    //カメラから前方向を取得
    Vector3 forward = m_camera->GetForward();

    //カメラから右方向を取得
    Vector3 right = m_camera->GetRight();

    //上下方向を無視するために0.0にする
    forward.y = 0.0f;
    right.y = 0.0f;

    //前方向と右方向を正規化
    forward.Normalize();
    right.Normalize();


    Vector3 moveDir = Vector3::Zero;

    // 入力処理
    //前進
    if (Input::IsKeyDown('W')) moveDir += forward;
    //後退（向きは変えずに後ろに歩く）
    if (Input::IsKeyDown('S')) moveDir -= forward;
    //右移動（ストライフ）
    if (Input::IsKeyDown('D')) moveDir += right;
    //左移動（ストライフ）
    if (Input::IsKeyDown('A')) moveDir -= right;

    if (moveDir.LengthSquared() > 0.0f)
    {
        //方向だけ必要なので正規化
        moveDir.Normalize();

        //移動距離 = 単位速度 × フレーム時間（1/60秒固定）
        Vector3 pos = GetOwner()->GetPosition();
        pos += moveDir * m_speed * (1.0f / 60.0f);
        GetOwner()->SetPosition(pos);

        // Wキーのみ、向きを徐々に変える
        if (Input::IsKeyDown('W'))
        {
            
            //Y軸の回転角を取得
            float targetYaw = std::atan2(moveDir.x, moveDir.z); 

            //今のプレイヤーの向きを取得
            Vector3 rot = GetOwner()->GetRotation();

            // 差分（ラップ処理）
            float delta = targetYaw - rot.y;
            while (delta > XM_PI) delta -= XM_2PI;
            while (delta < -XM_PI) delta += XM_2PI;

            // 徐々に回転
            rot.y += delta * m_rotateSpeed * (1.0f / 60.0f);
            GetOwner()->SetRotation(rot);
        }
    }

    std::cout << "Moveの更新は出来ています" << std::endl;
}
