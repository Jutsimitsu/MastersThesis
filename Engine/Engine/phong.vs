cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4x4 bones[42];
};

cbuffer CameraBuffer
{
	float3 eye;
};

struct VertexInputType
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	int4 bonesID : BONES;
	float4 weight : WEIGHTS;
	float hairLen : HAIRLENGTH;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
};

PixelInputType PhongShader(VertexInputType input)
{
	PixelInputType output;
	float4 worldPositionTranspose;

	input.position.w = 1.0f;
	
	float3 X = float3(input.position.x, input.position.y, input.position.z);

	float4x4 bone1 = bones[input.bonesID.x] * input.weight.x;
	float4x4 bone2 = bones[input.bonesID.y] * input.weight.y;
	float4x4 bone3 = bones[input.bonesID.z] * input.weight.z;
	float4x4 bone4 = bones[input.bonesID.w] * input.weight.w;

	bone1 = bone1+bone2+bone3+bone4;

	output.position = mul(float4(X,1), bone1);
	output.position = mul(output.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	worldPositionTranspose = mul(input.position, worldMatrix);

	output.viewDirection = eye.xyz - worldPositionTranspose.xyz;
	output.viewDirection = normalize(output.viewDirection);

	output.tex = input.tex;

	return output;
}