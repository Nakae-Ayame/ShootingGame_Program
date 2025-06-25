// VS の入力
struct VSInput
{
    float3 pos      : POSITION;
    float4 col      : COLOR;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

// VS から PS への出力
struct VSOutput
{
    float4 posH     : SV_POSITION;
    float4 col      : COLOR;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

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

VSOutput VSMain(VSInput vin)
{
    VSOutput output;
    float4 worldPos = mul(float4(vin.pos, 1), gWorld);
    float4 viewPos = mul(worldPos, gView);
    output.posH = mul(viewPos, gProj);

    output.col = vin.col;
    output.normal = normalize(mul(vin.normal, (float3x3) gWorld)); // ワールド空間法線
    output.texcoord = vin.texcoord;
    return output;
}


