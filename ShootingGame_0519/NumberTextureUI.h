#pragma once
#include <array>
#include <string>
#include <d3d11.h>
#include <SimpleMath.h>

class NumberTextureUI
{
public:
	//-----------Set関数------------
	void SetPosition(const DirectX::SimpleMath::Vector2& pos);
	void SetDigitSize(const DirectX::SimpleMath::Vector2& size);
	void SetSpacing(float spacing);

	//-----------Get関数------------
	DirectX::SimpleMath::Vector2 GetPosition() const;
	DirectX::SimpleMath::Vector2 GetDigitSize() const;

	//-----------数字関連関数------------
	void LoadDigitTextures(const std::string& folderPath);
	void DrawNumber(int value);

private:
	//---------表示設定関連--------------
	DirectX::SimpleMath::Vector2 m_position{ 30.0f, 30.0f };
	DirectX::SimpleMath::Vector2 m_digitSize{ 32.0f, 48.0f };
	float m_spacing = 2.0f;

	//---------テクスチャ関連--------------
	std::array<ID3D11ShaderResourceView*, 10> m_digitTextures{};
};
