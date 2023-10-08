struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return input.color;
}
