struct PS_Input
{
    float4 PosH : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 PSMain(PS_Input input) : SV_TARGET
{
    // 法線をカラーにして適当に色を出す
    float3 n = normalize(input.Normal);
    float3 col = abs(n);
    return float4(col, 1.0f);
}
