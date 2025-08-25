struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
};

//VS‚Ì“ü—Í
cbuffer WorldMatrix : register(b0)
{
    matrix gWorld;
};
cbuffer ViewMatrix : register(b1)
{
    matrix gView;
};
cbuffer ProjMatrix : register(b2)
{
    matrix gProjection;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

VSOutput VSMain(VSInput vin)
{
    VSOutput output;
    float4 posW = float4(vin.pos, 1.0f);
    output.posH = mul(posW, gWorld);
    output.posH = mul(output.posH, gView);
    output.posH = mul(output.posH, gProjection);
    output.uv = vin.uv;
    return output;
}