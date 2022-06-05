struct VS_INPUT
{
    float2 pos : POSITION;
	float2 texCoord : TEXCOORD;
};
            
struct PS_INPUT
{
    float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};
            
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos.xy, 0.0f, 1.0f);
	output.texCoord = input.texCoord;
    return output;
}