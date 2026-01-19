#include "EffectManager.h"
#include "GameObject.h"
#include "TextureManager.h"
#include "BillboardEffectComponent.h"
#include "BulletTrailComponent.h"
#include "renderer.h"

std::vector<std::shared_ptr<GameObject>> EffectManager::m_effectObjects;

void EffectManager::Init()
{
	m_effectObjects.clear();
}

void EffectManager::Update(float dt)
{
	for (auto& effect : m_effectObjects)
	{
		if (effect)
		{
			effect->Update(dt);
		}
	}
	RemoveFinishedEffects();
}

void EffectManager::Draw3D(float dt)
{
	for (auto& obj : m_effectObjects)
	{
		if (obj)
		{
			obj->Draw(dt);
		}
	}
}

void EffectManager::Uninit()
{
	m_effectObjects.clear();
}

void EffectManager::SpawnBillboardEffect(const BillboardEffectConfig& config,
								         const DirectX::SimpleMath::Vector3& pos)
{
	auto effe = std::make_shared<GameObject>();
	effe->SetPosition(pos);

	//ビルボードのエフェクトコンポーネントを追加
	auto fx = std::make_shared<BillboardEffectComponent>();
	fx->SetConfig(config);
	effe->AddComponent(fx);

	effe->Initialize();
	m_effectObjects.push_back(effe);
}

void EffectManager::SpawnExplosion(const DirectX::SimpleMath::Vector3& pos)
{
	BillboardEffectConfig config{};
	config.texturePath = "Asset/Effect/Effect_Explosion01.png"; 
	config.size = 30.0f;
	config.duration = 0.35f;
	config.cols = 3;
	config.rows = 3;
	config.isAdditive = true;
	config.color = DirectX::SimpleMath::Vector4(1, 1, 1, 1);

	SpawnBillboardEffect(config, pos);
}

void EffectManager::SpawnBulletTrail(const DirectX::SimpleMath::Vector3& startPos,
								     const DirectX::SimpleMath::Vector3& endPos)
{
	auto obj = std::make_shared<GameObject>();
	obj->SetPosition(startPos); 

	auto trail = std::make_shared<BulletTrailComponent>();
	trail->SetSegment(startPos, endPos);
	trail->SetWidth(2.0f);
	trail->SetDuration(0.5f);
	trail->SetAdditive(true);
	trail->SetColor(DirectX::SimpleMath::Vector4(1.0f, 0.0f, 1.0f, 0.8f));


	//配属先
	auto srv = TextureManager::Load("Asset/Effect/Bullet_Trail.png");

	trail->SetTexture(srv);

	obj->AddComponent(trail);
	obj->Initialize();

	m_effectObjects.push_back(obj);
}

void EffectManager::RemoveFinishedEffects()
{
	m_effectObjects.erase(
		std::remove_if(m_effectObjects.begin(), m_effectObjects.end(),
			[](const std::shared_ptr<GameObject>& obj)
			{
				if (!obj)
				{
					return true;
				}

				if (auto bb = obj->GetComponent<BillboardEffectComponent>())
				{
					return bb->IsFinished();
				}

				// 次にトレイル
				if (auto trail = obj->GetComponent<BulletTrailComponent>())
				{
					return trail->IsFinished();
				}

				return false;
			}),
		m_effectObjects.end());
}

