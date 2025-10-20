#pragma once
#include "GameObject.h"
#include "BulletComponent.h"
#include "Primitive.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class OBBColliderComponent; //前方宣言

class Bullet : public GameObject
{
public:

    Bullet() {};
    ~Bullet() override = default;

    //初期化
    void Initialize() override;

    //更新
    void Update(float dt) override;

    //描画
    void Draw(float alpha) override;

    //衝突判定
    void OnCollision(GameObject* other) override;

    //弾の半径のセッター
    void SetRadius(float r) { m_radius = r; }

private:
    //弾の半径
    float m_radius = 1.0f; 

    //弾の描画用
    Primitive m_primitive;  

    //コンポーネント保持
    std::shared_ptr<OBBColliderComponent> m_collider;   

};


