cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : UV;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : UV;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(pos, model);
	output.pos = pos;
    output.color = float4(input.color, 0.0f);
    output.uv = input.uv;

	return output;
}
