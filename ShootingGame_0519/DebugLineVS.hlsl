cbuffer MatrixBuffer : register(b0)
{
    float4x4 viewProj;
};

struct VS_IN
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VS_OUT VSMain(VS_IN input)  // ä÷êîñºÇ main Å® VSMain Ç…ïœçX
{
    VS_OUT o;
    o.pos = mul(float4(input.pos, 1.0f), viewProj);
    o.color = input.color;
    return o;
}
