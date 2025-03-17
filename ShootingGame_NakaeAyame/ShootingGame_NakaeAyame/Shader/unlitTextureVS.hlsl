#include "common.hlsl"

PS_IN vs_main(in VS_IN input)
{
    PS_IN output;
	
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);

    output.pos = mul(input.pos, wvp);
    float4 uv;
    uv.xy = input.tex; // s—ñŠ|‚¯Z‚Ì‚½‚ßfloat4Œ^‚ÉˆÚ‚· 
    uv.z = 0.0f; uv.w = 1.0f;
    uv = mul(uv, matrixTex); // UVÀ•W‚ÆˆÚ“®s—ñ‚ğŠ|‚¯Z 
    output.tex = uv.xy; // Š|‚¯Z‚ÌŒ‹‰Ê‚ğ‘—M—p•Ï”‚ÉƒZƒbƒg
    output.col = input.col;
    
    return output;
}

