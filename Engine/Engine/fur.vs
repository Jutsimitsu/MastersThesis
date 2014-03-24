cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix identityMatrix;
	float4x4 bones[42];
	float4x4 previousBones[42];
	float3 eye;
	float paddingMatrix;
};

cbuffer FurBuffer
{
	float FurLength;
	float3 Wind;
	float timer;
	float3 padding;
};

cbuffer Modifiers
{
	float gravityStrength;
	float stiffnessAmp;
	float dampeningAmp;
	float modpadding;
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
	float length : TEXCOORD1;
	float4 positionBase : TEXCOORD2;
	float4 positionEnd : TEXCOORD3;
	float3 viewDirection : TEXCOORD4;
};

static const float PI = 3.14159265f;

float random(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv ,float2(123.3124,421.233)*2.0)) * 1255.5453));
    return abs(noise.x + noise.y) * 0.5;
}

PixelInputType VS_FurShader(VertexInputType input) 
{
    PixelInputType output;

	float4x4 prev1 = previousBones[input.bonesID.x] * input.weight.x;
	float4x4 prev2 = previousBones[input.bonesID.y] * input.weight.y;
	float4x4 prev3 = previousBones[input.bonesID.z] * input.weight.z;
	float4x4 prev4 = previousBones[input.bonesID.w] * input.weight.w;

	prev1 = prev1+prev2+prev3+prev4;

	float4x4 bone1 = bones[input.bonesID.x] * input.weight.x;
	float4x4 bone2 = bones[input.bonesID.y] * input.weight.y;
	float4x4 bone3 = bones[input.bonesID.z] * input.weight.z;
	float4x4 bone4 = bones[input.bonesID.w] * input.weight.w;

	bone1 = bone1+bone2+bone3+bone4;

	float4 vertexPosition = mul(input.position, bone1);
	float4 movement = vertexPosition - mul(input.position, prev1);

	float3 gravity = float3(0, -10.0f*gravityStrength, 0);
	gravity = mul(gravity, viewMatrix);

	float windMovement = Wind.x * input.normal.x + Wind.y * input.normal.y + Wind.z * input.normal.z;
	float dampen = abs(windMovement)/(length(Wind)*length(input.normal)) + 1.0f;
	dampen = dampen*dampeningAmp;

	float fLength = pow((input.hairLen + input.hairLen*random(input.tex)/2.0f)/385.0f, 1.7f)*FurLength;
	float k =  pow(fLength, 3*stiffnessAmp);
	k = k;
	float3 transformedNormal = float3(input.normal.x+((random(input.tex)-0.5f)*pow((input.hairLen/255.0f),10)), input.normal.y+((random(input.tex)-0.5f)*pow((input.hairLen/255.0f),10)), input.normal.z+((random(input.tex)-0.5f))*(pow((input.hairLen/255.0f),10)));

	float3 X = transformedNormal * fLength;
	X = X + gravity*k;
	X = X + Wind*k*(1/dampen);
	X = X - movement.xyz * k * 50;
	
	//constraint hair lenght
	if(length(X)>fLength)
	{
		X = normalize(X) * fLength + vertexPosition.xyz;
	}
	else 
	{
		X = X + vertexPosition.xyz;
	}

	if( length(X-(input.position.xyz- input.normal*fLength))<fLength)
	{
		//X = (input.position.xyz - input.normal) + (normalize(X -(input.position.xyz - input.normal))*fLength); 
	}

	output.position = mul(float4(X, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	
	float4 worldPositionTranspose = mul(float4(X, 1.0f), worldMatrix);

	output.viewDirection = eye.xyz - worldPositionTranspose.xyz;
	output.viewDirection = normalize(output.viewDirection);

	output.positionBase = input.position;
	output.positionEnd = float4(X, 1.0f);
	output.tex = input.tex;
    output.normal = input.normal;
	output.length = FurLength;
    return output;
}