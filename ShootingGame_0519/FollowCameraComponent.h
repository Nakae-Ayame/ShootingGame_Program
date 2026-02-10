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
    //Vector3 GetPosition() const override { return m_spring.GetPosition(); }
    Vector3 GetAimPoint() const override;
    Vector2 GetReticleScreen() const { return m_reticleScreen; }
    Vector3 GetAimDirectionFromReticle() const;
    Vector3 GetUp() const;
	Vector3 GetLookTarget() const { return m_LookTarget; }
    Vector3 GetCachedAimPoint() const { return m_cachedAimPoint; }
    Vector3 GetCachedAimDir() const { return m_cachedAimDir; }
    bool GetHasCachedAim() const { return m_hasCachedAim; }
    Vector3 GetPosition() const override { return m_cameraWorldPos; }
    Vector3 GetShootRayOrigin() const { return m_shootRayOrigin; }
    Vector3 GetShootRayDir() const { return m_shootRayDir; }

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
    void UpdateShootCache();

    //-------追従対象関連--------
    GameObject* m_target = nullptr;           //追従対象
    PlayAreaComponent* m_playArea = nullptr;  //プレイ範囲


    //--------------射撃用レイ関連------------------
    Vector3 m_shootRayOrigin = Vector3::Zero;
    Vector3 m_shootRayDir = Vector3::Forward;

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
    
    //-------行列/スプリング関連-------
    SpringVector3 m_spring;     //カメラ位置をスプリングで滑らかに追従させるラッパー

    float m_normalFov = DirectX::XMConvertToRadians(50.0f);
    float m_boostFov  = DirectX::XMConvertToRadians(60.0f);

    float m_fovInSpeed  = 12.0f; 
    float m_fovOutSpeed =  1.0f; 

    float m_fovLerpSpeed = 8.0f;

    float m_normalStiffness = 35.0f;
    float m_normalDamping   = 22.0f;




    //-------レティクル/注視関連--------
    Vector2 m_reticleScreen{ 440.0f, 160.0f };

    Vector3 m_aimPoint = Vector3::Zero;            //レティクルが指すワールド座標
    float   m_aimPlaneDistance = 300.0f;              //レイと交差させる「カメラ前方の平面までの距離」

    float   m_verticalAimScale = 0.85f;

    float m_normalLookAheadDistance = 8.0f;
    float m_boostLookAheadDistance  = 15.0f;

    float m_lookAheadLerp     = 10.0f;   //lookTarget のスムーズ度合い
    float m_LookVerticalScale = 4.0f;    //値を大きくすると上下移動が派手になる
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

    //-------ブースト揺れ関連-------
    float m_boostShakeBlend = 0.0f;        // 0.0f:無し → 1.0f:最大
    float m_boostShakeInSpeed = 10.0f;      // フェードイン速度
    float m_boostShakeOutSpeed = 12.0f;    // フェードアウト速度
    float m_boostShakeMagnitude = 0.1f;   // 微振動の振幅（ワールド単位、要調整）
    float m_boostShakeFrequency = 28.0f;   // 微振動周波数（Hz、要調整）
    float m_boostShakePhase = 0.0f;

    //-------シェイク関連--------
    float m_shakeMagnitude = 0.0f;        //現在の振幅（ワールド単位）
    float m_shakeTimeRemaining = 0.0f;    //残り時間（秒）
    float m_shakeTotalDuration = 0.0f;    //最初に指定した振動時間（秒）
    float m_shakePhase = 0.0f;            //波形フェーズ
    float m_shakeFrequency = 25.0f;       //振動の基準周波数(Hz) — チューニング可

    Vector3 m_shakeOffset = DirectX::SimpleMath::Vector3::Zero;
    ShakeMode m_shakeMode = ShakeMode::Horizontal;  //現在の振動方向を決めるモード

    //--------------射撃照準キャッシュ関連------------------
    Vector3 m_cachedAimPoint = Vector3::Zero;   // 次フレームで射撃が使う照準点
    Vector3 m_cachedAimDir = Vector3::Forward;// 次フレームで射撃が使う照準方向（正規化）
    bool    m_hasCachedAim = false;           // 初期化直後などの安全用

    Vector3 m_cameraWorldPos = Vector3::Zero;
};