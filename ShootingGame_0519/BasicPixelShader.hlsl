// ピクセルシェーダー入力：クリップ座標 + 頂点カラー
struct PSInput
{
    float4 posH : SV_POSITION;
    float4 col : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    // 頂点カラーをそのまま出力
    return input.col;
}
