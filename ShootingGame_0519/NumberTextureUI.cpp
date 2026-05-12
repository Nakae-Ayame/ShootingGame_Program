#include "NumberTextureUI.h"
#include "TextureManager.h"
#include "renderer.h"

using namespace DirectX::SimpleMath;

void NumberTextureUI::SetPosition(const Vector2& pos)
{
	m_position = pos;
}

void NumberTextureUI::SetDigitSize(const Vector2& size)
{
	m_digitSize = size;
}

void NumberTextureUI::SetSpacing(float spacing)
{
	m_spacing = spacing;
}

Vector2 NumberTextureUI::GetPosition() const
{
	return m_position;
}

Vector2 NumberTextureUI::GetDigitSize() const
{
	return m_digitSize;
}

void NumberTextureUI::LoadDigitTextures(const std::string& folderPath)
{
	for (int i = 0; i < 10; ++i)
	{
		std::string path = folderPath + "/" + std::to_string(i) + ".png";
		m_digitTextures[i] = TextureManager::Load(path);
	}
}

void NumberTextureUI::DrawNumber(int value)
{
	if (value < 0)
	{
		value = 0;
	}

	std::string numberText = std::to_string(value);

	for(int i = 0; i < static_cast<int>(numberText.size()); ++i)
	{
		char c = numberText[i];

		if(c < '0' || c > '9'){ continue; }//数字以外の文字はスキップ

		int digit = c - '0';

		ID3D11ShaderResourceView* texture = m_digitTextures[digit];

		if (!texture) { continue; }//テクスチャがない場合はスキップ

		Vector2 drawPos = m_position;

		drawPos.x += static_cast<float>(i) * (m_digitSize.x + m_spacing);

		Renderer::DrawTexture(texture, drawPos, m_digitSize);
	}
}