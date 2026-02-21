#pragma once
#include <SimpleMath.h>
#include <vector>
#include <memory>
#include "commontypes.h" 
#include "Model.h"       
#include "Component.h"
#include "IScene.h"

class Component;

class GameObject
{
public:
    GameObject() = default;
    virtual ~GameObject() = default;

    virtual void Initialize();
    virtual void Update(float dt);   
    virtual void Draw(float alpha); 
    virtual void Uninit();

    //--------Set関数-------
    void SetPosition(const Vector3& pos) { m_transform.pos = pos;}
    void SetRotation(const Vector3& rot) { m_transform.rot = rot; }
    void SetScale(const Vector3& scl) { m_transform.scale = scl; }
    void SetScene(IScene* s) { m_scene = s; }

    //--------Get関数-------
    const Vector3& GetPosition() { return m_transform.pos; }
    const Vector3& GetPrevPosition() const { return m_prevPosition; }
    const Vector3& GetRotation() { return m_transform.rot; }
    const Vector3& GetScale()    { return m_transform.scale;}
    IScene* GetScene() const { return m_scene; }

    //--------Component関連-------

    void AddComponent(std::shared_ptr<Component> comp);

    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args)
    {
        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        AddComponent(comp);
        return comp;
    }

    //現在シーンのゲッター・セッター

    //まとめてトランスフォームのゲッターセッター
    const SRT& GetTransform() const { return m_transform; }

    //
    DirectX::SimpleMath::Matrix  GetWorldMatrix() const;  //ワールド変換行列を返す
    DirectX::SimpleMath::Vector3 GetForward() const;      //ワールド前方(正規化済み)
    DirectX::SimpleMath::Vector3 GetRight() const;        //ワールド右方向(正規化済み)
    DirectX::SimpleMath::Vector3 GetUp() const;           //ワールド上方向(正規化済み)

    //衝突通知
    virtual void OnCollision(GameObject* other) {}

    template<typename T>
    std::shared_ptr<T> GetComponent() const
    {
        for (auto& comp : m_components)
        {
            auto casted = std::dynamic_pointer_cast<T>(comp);
            if (casted)
            {
                return casted;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<Component>> m_components;

    bool m_uninitialized = false;
    SRT m_transform;
    GameObject* m_parent = nullptr; // 親オブジェクト（親がいない場合は nullptr）]
    SRT m_prevTransform; // ← 補間用に追加
    Vector3 m_prevPosition = Vector3::Zero;
    IScene* m_scene = nullptr;
};