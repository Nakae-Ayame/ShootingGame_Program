cbuffer WorldBuffer : register(b0)
{
	matrix World;
}
cbuffer ViewBuffer : register(b1)
{
	matrix View;
}
cbuffer ProjectionBuffer : register(b2)
{
	matrix Projection;
}

struct VS_IN
{
    float4 pos : POSITION0;
	float4 nrm : NORMAL0;
    float4 col : COLOR0;
    float2 tex : TEXCOORD0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 tex : TEXCOORD0;
};

struct LIGHT
{
    bool Enable;
    bool3 Dummy; //バイト数的にキリを付けるために入れている
    float4 Direction; //平行光源の方向
    float4 Diffuse; //並行光源の強さと色
    float4 Ambient; //環境光の強さと色
};

cbuffer LightBuffer : register(b3)
{
   LIGHT Light;
}

struct MATERIAL 
{ 
    float4 Ambient; 
    float4 Diffuse; 
    float4 Specular;
    float4 Emission; 
    float Shininess; 
    bool TextureEnable; 
    bool2 Dummy; 
}; 
    
cbuffer MaterialBuffer : register(b4) 
{ 
    MATERIAL Material; 
}

// UV座標移動行列 
cbuffer MaterialBuffer : register(b5) 
{
    matrix matrixTex;
}

