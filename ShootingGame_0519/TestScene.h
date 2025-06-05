#pragma once
#include <vector>
#include "IScene.h"
#include "renderer.h"

class TestScene : public IScene
{
private:
	ComPtr<ID3D11Buffer> m_VB;  // 頂点バッファ
	ComPtr<ID3D11Buffer> m_IB;  // インデックスバッファ
	UINT m_IndexCount = 0;

public:
	explicit TestScene() {};
	//~TestScene() {};
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
	void Init() override;
	void Uninit() override;
private:
};