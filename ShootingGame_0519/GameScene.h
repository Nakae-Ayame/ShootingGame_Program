#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"
#include "FreeCamera.h"
#include "renderer.h"
#include "Model.h"

//---------------------------------
//ISceneを継承したGameScene
//---------------------------------
class GameScene : public IScene
{
public:
	explicit GameScene() {};
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
	void Init() override;
	void Uninit() override;
private:
	/*ComPtr<ID3D11Buffer> m_VB; 
	ComPtr<ID3D11Buffer> m_IB;
	UINT m_IndexCount = 0;*/
	FreeCamera m_FreeCamera;
	std::unique_ptr<Model> m_Model;   // モデル保持用ポインタ
};