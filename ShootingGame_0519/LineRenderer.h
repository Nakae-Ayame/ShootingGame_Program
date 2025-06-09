#pragma once

class LineRenderer
{
private:
    ComPtr<ID3D11Buffer> m_VertexBuffer;
    ComPtr<ID3D11VertexShader> m_VertexShader;
    ComPtr<ID3D11PixelShader> m_PixelShader;
    ComPtr<ID3D11InputLayout> m_InputLayout;

    struct LineVertex
    {
        Vector3 Position;
        Color Color;
    };

    std::vector<LineVertex> m_Lines;

public:
    void Init();
    void Uninit();
    void Draw(const Matrix4x4& view, const Matrix4x4& proj);
    void AddLine(const Vector3& start, const Vector3& end, const Color& color);
    void ClearLines(); // 毎フレームクリアする想定（デバッグ描画など）
};

