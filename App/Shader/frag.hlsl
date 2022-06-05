struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D colorMap: register(t0);
SamplerState colorMapSampler: register(s0);

float4 main(PS_INPUT vertexOut) : SV_Target
{
	float4 color = colorMap.Sample(colorMapSampler, vertexOut.texCoord);
	return color;
}