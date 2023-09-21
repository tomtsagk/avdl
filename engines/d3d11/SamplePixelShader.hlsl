Texture2D mytexture;
SamplerState mysampler;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 outputTex = mytexture.Sample(mysampler, input.uv);
    //if (outputTex.a < 0.02f)
    //    discard;
    float4 output = float4(input.color) +outputTex;
    return float4(output.rgb, max(outputTex.a, input.color.a));
}
