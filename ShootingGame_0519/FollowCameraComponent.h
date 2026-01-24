#pragma once
#include "SpringVector3.h"
#include "CameraComponentBase.h"
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <algorithm>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class GameObject;
class PlayAreaComponent;
class MoveComponent;

class FollowCameraComponent : public CameraComponentBase
{
public:

    FollowCameraComponent();

    void Initialize() override {};
    void Update(float dt) override;

    //--------Set関数-------
    void SetTarget(GameObject* target);
    void SetPlayArea(PlayAreaComponent* p) { m_playArea = p; }
    void SetVerticalAimScale(float s) { m_verticalAimScale = std::clamp(s, 0.0f, 1.0f); }
    void SetDistance(float dist) { m_defaultDistance = dist; }    
    void SetHeight(float h) { m_defaultHeight = h; }
    void SetSensitivity(float s) { m_sensitivity = s; }
    void SetBoostState(bool isBoosting) override { m_boostRequested = isBoosting; };
    void SetReticleScreen(const Vector2& center){ m_reticleScreen = center; };

    //--------Get関数-------
    float GetVerticalAimScale() const { return m_verticalAimScale; }
    float GetSensitivity() const { return m_sensitivity; }
    Matrix GetView() const { return m_ViewMatrix; }
    Matrix GetProj() const { return m_ProjectionMatrix; }
    Vector3 GetForward() const override{ return m_ViewMatrix.Invert().Forward(); }
    Vector3 GetRight() const override{ return m_ViewMatrix.Invert().Right(); }
    Vector3 GetPosition() const override { return m_spring.GetPosition(); }
    Vector3 GetAimPoint() const override;
    Vector2 GetReticleScreen() const { return m_reticleScreen; }
    Vector3 GetAimDirectionFromReticle() const;

    //-------カメラ演出--------
    enum class ShakeMode
    {
        Horizontal,   
        Vertical,     
        ALL
    };
    
    void Shake(float magnitude, float duration, ShakeMode mode = ShakeMode::Horizontal);
private:

    //-------更新関連関数--------
    void UpdateCameraPosition(float dt);
    void UpdateLookTarget(float dt,const Vector3& cameraPos);
    void UpdateAimPoint(float dt, const Vector3& cameraPos);
    void UpdateShake(float dt, const Vector3& cameraPos);
    void UpdateProjectionMatrix();
    void UpdateFov(float dt);

    void ComputeReticleRay(const Matrix& view, Vector3* outOrigin, Vector3* outDir) const;
    Vector3 ComputeRayPlaneIntersection(const Vector3& rayOrigin,
                                        const Vector3& rayDir,
                                        const Vector3& planePoint,
                                        const Vector3& planeNormal) const;

    //-------追従対象関連--------
    GameObject* m_target = nullptr;           //追従対象
    PlayAreaComponent* m_playArea = nullptr;  //プレイ範囲

    //-------入力/回転関連--------
    float m_yaw = 0.0f;               //回転角(ヨー)
    float m_pitch = 0.0f;             //回転角(ピッチ)
    float m_sensitivity = 0.001f;     //回る量

    float m_pitchLimitMin = XMConvertToRadians(-15.0f); //ピッチの制限値
    float m_pitchLimitMax = XMConvertToRadians( 45.0f); //ピッチの制限値
    float m_yawLimit      = XMConvertToRadians(120.0f); //ヨーの制限値

    //-------カメラ距離/高さ関連--------
    float m_defaultDistance = 0.05f;    //追従対象の後方にどのぐらいにカメラがいるのか
    float m_defaultHeight   = 3.5f;     //追従対象からどのぐらい高い所にカメラがいるのか

    float m_aimDistance = 0.004f;      //エイムしたときの後方どのあたりにカメラがいるのか
    float m_aimHeight   = 2.8f;         //エイムしたとき追従対象からどのぐらい高い所にカメラがいるのか

    bool m_isAiming = false;          //今エイムしているかどうかのbool型
    bool m_isBoosting = false;
    
    //-------行列/スプリング関連--------
    Matrix m_viewsMatrix;        //ビュー行列
    Matrix m_projectionMatrix;  //プロジェクト行列
    SpringVector3 m_spring;     //カメラ位置をスプリングで滑らかに追従させるラッパー

    float m_normalFov = DirectX::XMConvertToRadians(50.0f);
    float m_boostFov  = DirectX::XMConvertToRadians(60.0f);

    float m_fovInSpeed  = 12.0f; 
    float m_fovOutSpeed = 6.0f; 

    float m_fovLerpSpeed = 8.0f;

    float m_normalStiffness = 35.0f;
    float m_normalDamping   = 22.0f;

    //-------レティクル/注視関連--------
    Vector2 m_reticleScreen{ 440.0f, 160.0f };

    Vector3 m_aimPoint = Vector3::Zero;            //レティクルが指すワールド座標
    float   m_aimPlaneDistance = 300.0f;              //レイと交差させる「カメラ前方の平面までの距離」

    float   m_verticalAimScale = 0.85f;

    float   m_lookAheadDistance = 8.0f;    // どれだけ前方（レティクル方向）を注視するか（画面内でプレイヤーをずらす量）
    float   m_lookAheadLerp     = 10.0f;       // lookTarget のスムーズ度合い
    float   m_LookVerticalScale = 4.0f;   // 値を大きくすると上下移動が派手になる
    Vector3 m_LookTarget = Vector3::Zero;

    //-------ターン関連--------
    float m_prevPlayerYaw   = 0.0f;
    float m_turnOffsetScale = 8.0f;   // yawSpeed -> ワールド横オフセット換算（調整用）
    float m_turnOffsetMax   = 12.0f;     // オフセット最大値（ワールド単位）
    float m_turnOffsetLerp  = 6.0f;    // オフセットが変化するときの滑らかさ（大きいと即時）
    float m_currentTurnOffset = 0.0f; // 現在の横オフセット（滑らかに更新）
    
    float m_screenOffsetScale = 20.0f; // 画面幅 1.0 正規化あたりのワールド単位換算（調整可）
    float m_maxScreenOffset   = 24.0f;  // 最大シフト（ワールド単位）

    //-------ブースト関連-------
    bool m_boostRequested = false;   // 現在ボタンでブースト要求中か（MoveComponent から SetBoostState）
    float m_boostDistanceMul = 0.85f;  // ブースト中は距離を 85% にする（近づく）

    //-------シェイク関連--------
    float m_shakeMagnitude = 0.0f;        //現在の振幅（ワールド単位）
    float m_shakeTimeRemaining = 0.0f;    //残り時間（秒）
    float m_shakeTotalDuration = 0.0f;    //最初に指定した振動時間（秒）
    float m_shakePhase = 0.0f;            //波形フェーズ
    float m_shakeFrequency = 25.0f;       //振動の基準周波数(Hz) — チューニング可

    Vector3 m_shakeOffset = DirectX::SimpleMath::Vector3::Zero;
    ShakeMode m_shakeMode = ShakeMode::Horizontal;  //現在の振動方向を決めるモード
};