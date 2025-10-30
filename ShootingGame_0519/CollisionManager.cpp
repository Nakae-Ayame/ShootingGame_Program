#include <iostream>
#include <DirectXMath.h>
#include "CollisionManager.h"
#include "Collision.h" // IsAABBHit, IsOBBHit, IsAABBvsOBB
#include "DebugGlobals.h"
#include "renderer.h"
#include "GameObject.h"

using namespace DirectX;

std::vector<ColliderComponent*> CollisionManager::m_Colliders;
bool CollisionManager::m_hitThisFrame = false;

void CollisionManager::RegisterCollider(ColliderComponent* collider)
{
    if (!collider)
    {
        return;
    }

    //オーナーがセットされているかのチェック
    if (!collider->GetOwner())
    {
        //ログ表示
        std::cout << "【警告】コライダーの所持者が存在しません" << std::endl;
        return;
    }

    //重複防止
    if (std::find(m_Colliders.begin(), m_Colliders.end(), collider) == m_Colliders.end())
    {
        m_Colliders.push_back(collider);
    }
}

void CollisionManager::UnregisterCollider(ColliderComponent* collider)
{
    if (!collider)
    {
        return;
    }

    auto it = std::find(m_Colliders.begin(), m_Colliders.end(), collider);
    if (it != m_Colliders.end())
    {
        m_Colliders.erase(it);
    }
}

void CollisionManager::Clear()
{
    m_Colliders.clear();
}

void CollisionManager::CheckCollisions()
{
    //全コライダーを未ヒット状態に
    for (auto col : m_Colliders) 
    {
        col->SetHitThisFrame(false);
    }

    //判定する物の収集を行う
    std::vector<CollisionInfoLite> hitPairs;
    size_t count = m_Colliders.size();
       
    for (size_t i = 0; i < count; ++i)
    {
        ColliderComponent* colA = m_Colliders[i];
        if(!colA)
        {
            continue;//コライダーが付いていなければ
        }

        GameObject* ownerA = colA->GetOwner();
        if(!ownerA)
        {
            continue;// 所有者が無ければスキップ（安全）
        }

        for (size_t j = i + 1; j < count; ++j)
        {
            ColliderComponent* colB = m_Colliders[j];
            if (!colB)
            {
                continue;
            }

            GameObject* ownerB = colB->GetOwner();
            if (!ownerB)
            {
                continue;
            }
            
            bool hit = false;

            //コライダーの種類(AABB or OBB)を取得
            auto typeA = colA->GetColliderType();
            auto typeB = colB->GetColliderType();

            //-----------------------------------------
            // 衝突判定 ： AABB vs AABB
            //-----------------------------------------
            if (typeA == ColliderType::AABB && typeB == ColliderType::AABB)
            {
                auto a = static_cast<AABBColliderComponent*>(colA);
                auto b = static_cast<AABBColliderComponent*>(colB);
                hit = Collision::IsAABBHit(a->GetMin(), a->GetMax(), b->GetMin(), b->GetMax());
            }
            //-----------------------------------------
            // 衝突判定 ： OBB vs OBB
            //-----------------------------------------
            else if (typeA == ColliderType::OBB && typeB == ColliderType::OBB)
            {
                auto a = static_cast<OBBColliderComponent*>(colA);
                auto b = static_cast<OBBColliderComponent*>(colB);

                //各データを取得する
                Vector3 centerA = a->GetCenter();
                Vector3 centerB = b->GetCenter();
                Matrix rotA = a->GetRotationMatrix();
                Matrix rotB = b->GetRotationMatrix();
                Vector3 halfA = a->GetSize() * 0.5f;
                Vector3 halfB = b->GetSize() * 0.5f;

                //
                Vector3 axesA[3] = { rotA.Right(), rotA.Up(), rotA.Forward() };
                Vector3 axesB[3] = { rotB.Right(), rotB.Up(), rotB.Forward() };

                hit = Collision::IsOBBHit(centerA, axesA, halfA, centerB, axesB, halfB);
            }

            //-----------------------------------------
            // 衝突判定 ： AABB vs OBB
            //-----------------------------------------
            else
            {
                AABBColliderComponent* aabb = nullptr;
                OBBColliderComponent* obb = nullptr;
                if (typeA == ColliderType::AABB)
                {
                    aabb = static_cast<AABBColliderComponent*>(colA);
                    obb  = static_cast<OBBColliderComponent*>(colB); 
                }
                else
                {
                    aabb = static_cast<AABBColliderComponent*>(colB); 
                    obb  = static_cast<OBBColliderComponent*>(colA);
                }

                // AABB と OBB 双方の Get* は null-safe 実装を期待
                hit = Collision::IsAABBvsOBBHit(aabb->GetMin(), aabb->GetMax(), obb->GetCenter(), obb->GetRotationMatrix(), obb->GetSize() * 0.5f);
            }

            //-----------------------------------------
            // 結果を送る(ログも出す)
            //-----------------------------------------
            if (hit)
            {
                //当たっている場合のログ
                //std::cout << "当たっています" << std::endl;

                // コリジョンイベント通知
                //判定フェーズでは通知しないで入れておく。
                CollisionInfoLite info;
                info.a = colA;
                info.b = colB;
                hitPairs.push_back(info);
            }
            else
            {
                //当たっていない場合のログ（デバッグ時のみ有効にするといい）
                //std::cout << "当たっていません "<< std::endl;
            }

            for (const auto& p : hitPairs)
            {
                if (!p.a || !p.b)
                {
                    //一つ目の当たり判定と二つ目の当たり判定の中で
                    // いずれかが無かったら終了
                    continue;
                }

                ColliderComponent* colA = p.a;
                ColliderComponent* colB = p.b;

                GameObject* ownerA = colA->GetOwner();
                GameObject* ownerB = colB->GetOwner();

                if (!ownerA || !ownerB) continue; // 二度目の安全チェック

                // Set Hit flag
                colA->SetHitThisFrame(true);
                colB->SetHitThisFrame(true);

                // 通知。既存のシグネチャを使う場合は null チェックのみ。
                ownerA->OnCollision(ownerB);
                ownerB->OnCollision(ownerA);

                // もし新しい CollisionInfo 構造体を使うならここで構築して渡す
            }
        }
    }

}

void CollisionManager::DebugDrawAllColliders(DebugRenderer& dr)
{
    if (m_Colliders.empty()) 
    {
        std::cout << "コライダーに何も登録されていません " << std::endl;
        return;
    }

    // 深度オフにして両面描画で見やすく
    //m_DeviceContextBackup... // （もし保存できるなら保存、無ければ Renderer 関数で切り替え）
    Renderer::SetDepthEnable(false);
    Renderer::DisableCulling(false);

    for (auto* col : m_Colliders)
    {
        if (!col) continue;
        bool hit = col->IsHitThisFrame();
        Vector4 color = hit ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 0.7f);

        if (col->GetColliderType() == ColliderType::AABB)
        {
            auto* a = static_cast<AABBColliderComponent*>(col);
            Vector3 mn = a->GetMin();
            Vector3 mx = a->GetMax();
            Vector3 center = (mn + mx) * 0.5f;
            Vector3 fullSize = (mx - mn);
            Vector3 halfSize = fullSize * 0.5f;

            char buf[256];
            sprintf_s(buf, "DBG: AABB center=(%f,%f,%f) fullSize=(%f,%f,%f)\n",
                center.x, center.y, center.z, fullSize.x, fullSize.y, fullSize.z);
            OutputDebugStringA(buf);

            fullSize = halfSize * 2.0f;
            dr.AddBox(center, fullSize, Matrix::Identity, color);
        }
        else // OBB
        {
            auto* o = static_cast<OBBColliderComponent*>(col);
            Vector3 center = o->GetCenter();
            Vector3 fullSize = o->GetSize();
            Vector3 halfSize = fullSize * 0.5f;
            Matrix rot = o->GetRotationMatrix();

            char buf[256];
            sprintf_s(buf, "DBG: OBB center=(%f,%f,%f) fullSize=(%f,%f,%f)\n",
                center.x, center.y, center.z, fullSize.x, fullSize.y, fullSize.z);
            OutputDebugStringA(buf);

            dr.AddBox(center, halfSize, rot, color);
        }
    }

    // 元に戻す
    Renderer::SetDepthEnable(true);
    Renderer::DisableCulling(true);
}


//void CollisionManager::DebugDrawAllColliders(DebugRenderer& dr)
//{
//    // 見えやすく一時的にデプスオフ、カリングオフ
//    Renderer::SetDepthEnable(false);
//    Renderer::DisableCulling(false);
//
//    for (auto* col : m_Colliders)
//    {
//        bool hit = col->IsHitThisFrame();
//        Vector4 color = hit ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 0.7f);
//
//        if (col->GetColliderType() == ColliderType::AABB)
//        {
//            auto* a = static_cast<AABBColliderComponent*>(col);
//            Vector3 mn = a->GetMin();
//            Vector3 mx = a->GetMax();
//            Vector3 center = (mn + mx) * 0.5f;
//            Vector3 fullSize = (mx - mn);
//            Vector3 halfSize = fullSize * 0.5f;
//
//            char buf[256];
//            sprintf_s(buf, "DBG: AABB center=(%f,%f,%f) fullSize=(%f,%f,%f)\n",
//                center.x, center.y, center.z, fullSize.x, fullSize.y, fullSize.z);
//            OutputDebugStringA(buf);
//
//            dr.AddBox(center, halfSize, Matrix::Identity, color);
//        }
//        else // OBB
//        {
//            auto* o = static_cast<OBBColliderComponent*>(col);
//            Vector3 center = o->GetCenter();
//            Vector3 fullSize = o->GetSize();
//            Vector3 halfSize = fullSize * 0.5f;
//            Matrix rot = o->GetRotationMatrix();
//
//            char buf[256];
//            sprintf_s(buf, "DBG: OBB center=(%f,%f,%f) fullSize=(%f,%f,%f)\n",
//                center.x, center.y, center.z, fullSize.x, fullSize.y, fullSize.z);
//            OutputDebugStringA(buf);
//
//            dr.AddBox(center, halfSize, rot, color);
//        }
//    }
//
//    // restore
//    Renderer::SetDepthEnable(true);
//    Renderer::DisableCulling(true);
//}



