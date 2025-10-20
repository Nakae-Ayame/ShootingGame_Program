#include <DirectXMath.h>
#include <iostream>
#include "FollowCameraComponent.h"
#include "Renderer.h"
#include "Application.h"
#include "Input.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

FollowCameraComponent::FollowCameraComponent()
{
    // スプリング設定
    m_Spring.SetStiffness(12.0f);
    m_Spring.SetDamping(6.0f);
    m_Spring.SetMass(1.0f);

    // プロジェクション行列
    float width = static_cast<float>(Application::GetWidth());
    float height = static_cast<float>(Application::GetHeight());
    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f), width / height, 0.1f, 1000.0f);
}

//カメラが追う対象をセットする関数
void FollowCameraComponent::SetTarget(GameObject* target)
{
    m_Target = target;
    if (target)
    {
        //ターゲット位置 + (高さ, 後方) のオフセットで初期カメラ位置を決める
        Vector3 initial = target->GetPosition() + Vector3(0, m_DefaultHeight, -m_DefaultDistance);
        m_Spring.Reset(initial);
    }
}

//
void FollowCameraComponent::Update(float dt)
{
    //ターゲットがいないなら処理しない
    if (!m_Target) return;

    //エイム（右クリックのみ）
    m_IsAiming = Input::IsMouseRightDown();

    //マウスによる視点制御
    POINT delta = Input::GetMouseDelta();

    //感度を考慮して角度に反映
    m_Yaw += delta.x * m_Sensitivity;
    m_Pitch += delta.y * m_Sensitivity;

    //角度制限（ラジアン）
    m_Pitch = std::clamp(m_Pitch, m_PitchLimitMin, m_PitchLimitMax);
    m_Yaw = std::clamp(m_Yaw, -m_YawLimit, m_YawLimit);

    //カメラ位置（プレイヤーの背後）をスプリングに与える
    UpdateCameraPosition(dt);

    //カメラの現在位置（スプリングから取得）
    Vector3 cameraPos = m_Spring.GetPosition();
    Vector3 targetPos = m_Target->GetPosition();

    //Unprojectを使ってレティクルのワールド点を計算
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());

    float sx = m_ReticleScreen.x;
    float sy = m_ReticleScreen.y;

    //画面空間からオブジェクト空間に落とし込んで場所を仮で決める
    Matrix provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);

    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&provisionalView));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);

    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);

    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    Vector3 rayDir = farWorld - nearWorld;
    if (rayDir.LengthSquared() > 1e-6f) rayDir.Normalize();

    // カメラ前方の平面(法線=camForward)との交差を求める
    Vector3 camForward = GetForward();
    Vector3 planePoint = cameraPos + camForward * m_AimPlaneDistance;
    float denom = rayDir.Dot(camForward);

    Vector3 worldTarget;
    const float EPS = 1e-5f;
    if (fabs(denom) < EPS)
    {
        // 平行に近い場合は近点から ray を伸ばす
        worldTarget = nearWorld + rayDir * m_AimPlaneDistance;
    }
    else
    {
        float t = (planePoint - nearWorld).Dot(camForward) / denom;
        worldTarget = nearWorld + rayDir * t;
    }

    // AimPoint を滑らかに更新（直接代入だと振動しやすい）
    const float aimLerp = 12.0f; // 追従の速さ（調整可）
    m_AimPoint = m_AimPoint + (worldTarget - m_AimPoint) * std::min(1.0f, aimLerp * dt);

    // ------- lookTarget（カメラの注視点）を m_AimPoint に寄せて計算 -------
    Vector3 aimDir = m_AimPoint - targetPos;
    if (aimDir.LengthSquared() > 1e-6f) aimDir.Normalize();
    else
    {
        // フォールバック：現在のカメラの前方を使う（安全策）
        aimDir = GetForward();
    }

    // 前方へどれだけ注視点を置くか（この値でプレイヤーの画面内位置が変わる）
    Vector3 rawLookTarget = targetPos + aimDir * m_LookAheadDistance + Vector3(0.0f, (m_DefaultHeight + m_AimHeight) * 0.5f, 0.0f);

    // 平滑化（首の挙動）
    m_LookTarget = m_LookTarget + (rawLookTarget - m_LookTarget) * std::min(1.0f, m_LookAheadLerp * dt);

    // ビュー行列はカメラの位置 -> 平滑化された注視点
    m_ViewMatrix = Matrix::CreateLookAt(cameraPos, m_LookTarget, Vector3::Up);

    // レンダラへセット
    Renderer::SetViewMatrix(m_ViewMatrix);
    Renderer::SetProjectionMatrix(m_ProjectionMatrix);
}


void FollowCameraComponent::UpdateCameraPosition(float dt)
{
    // 距離と高さはエイムでのみ変化（右クリック）
    float dist;
    float height;
    if (m_IsAiming)
    {
        dist = m_AimDistance;
        height = m_AimHeight;
    }
    else
    {
        dist = m_DefaultDistance;
        height = m_DefaultHeight;
    }

    //カメラ位置をプレイヤーの背後に置く
    Vector3 targetPos = m_Target->GetPosition(); //Playerのポジションを取ってくる
    Vector3 targetRot = m_Target->GetRotation(); //Playerの回転を取ってくる
    float playerYaw = targetRot.y;          //(rot.y = yaw を想定)

    Matrix  playerRot = Matrix::CreateRotationY(playerYaw); //Y軸周りに回転行列を作る
    Vector3 baseOffset = Vector3(0.0f, 0.0f, -dist);         //プレイヤーのローカル座標系で見た時のカメラの位置
    Vector3 rotatedOffset = Vector3::Transform(baseOffset, playerRot);  //回転座標をカメラに設定する

    Vector3 desiredPos = targetPos + rotatedOffset + Vector3(0.0f, height, 0.0f);

    // --- ここから追加：レティクルのスクリーン X に基づく横シフトを適用 ---
    // m_ReticleScreen はクライアント座標 (px)
    float screenW = static_cast<float>(Application::GetWidth());
    // 中心基準にして [-1..1] に正規化
    float normX = 0.0f;
    if (screenW > 1.0f)
    {
        normX = (m_ReticleScreen.x - (screenW * 0.5f)) / (screenW * 0.5f); // -1 .. 1
    }

    // カメラ右方向（プレイヤーの向きに合わせた右）を計算
    Vector3 localRight = Vector3::Transform(Vector3::Right, playerRot);
    // 横オフセット量（ワールド単位）を決定（符号そのままでOK）
    float lateral = normX * m_ScreenOffsetScale;
    // 制限
    lateral = std::clamp(lateral, -m_MaxScreenOffset, m_MaxScreenOffset);

    // desiredPos に右方向のオフセットを追加（レティクル右 -> カメラ右へ移動）
    desiredPos += localRight * lateral;

    // --- ここまで追加 ---

    //スプリングで位置を滑らかに追従させる
    m_Spring.Update(desiredPos, dt);
}

Vector3 FollowCameraComponent::GetForward() const
{
    return m_ViewMatrix.Invert().Forward();
}

Vector3 FollowCameraComponent::GetRight() const
{
    return m_ViewMatrix.Invert().Right();
}

Vector3 FollowCameraComponent::GetAimDirectionFromReticle() const
{
    // safety checks
    if (!m_Target) {
        // fallback: camera forward
        return GetForward();
    }

    // camera pos and provisional view (same as in Update)
    Vector3 cameraPos = m_Spring.GetPosition();
    Vector3 targetPos = m_Target->GetPosition();
    Matrix provisionalView = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);

    // screen coordinates from stored reticle (client coords)
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    float sx = m_ReticleScreen.x;
    float sy = m_ReticleScreen.y;

    // build matrices for XM functions
    XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_ProjectionMatrix));
    XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&provisionalView));
    XMMATRIX worldXM = XMMatrixIdentity();

    XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
    XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

    XMVECTOR nearWorldV = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);

    XMVECTOR farWorldV = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, screenW, screenH,
        0.0f, 1.0f,
        projXM,
        viewXM,
        worldXM);

    Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
    Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

    Vector3 dir = farWorld - nearWorld;
    if (dir.LengthSquared() > 1e-6f) dir.Normalize();
    else dir = GetForward(); // fallback

    return dir;
}
