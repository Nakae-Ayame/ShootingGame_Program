Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSInput
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return tex.Sample(samp, input.uv);
}
