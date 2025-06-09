// 定数バッファ
cbuffer WorldBuffer : register(b0)
{
    matrix gWorld;
}
cbuffer ViewBuffer : register(b1)
{
    matrix gView;
}
cbuffer ProjBuffer : register(b2)
{
    matrix gProj;
}

// 頂点シェーダー入力：位置 + 頂点カラー
struct VSInput
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

// ピクセルシェーダー入力：クリップ座標 + 頂点カラー
struct PSInput
{
    float4 posH : SV_POSITION;
    float4 col : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // ワールド→ビュー→プロジェクション変換
    float4 worldPos = mul(float4(input.pos, 1.0f), gWorld);
    float4 viewPos = mul(worldPos, gView);
    output.posH = mul(viewPos, gProj);

    // 頂点カラーをそのまま渡す
    output.col = input.col;
    return output;
}

