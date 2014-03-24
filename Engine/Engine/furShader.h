#pragma once

#ifndef _FURSHADERCLASS_H_
#define _FURSHADERCLASS_H_

#include <D3D11.h>
#include <D3DX10math.h>
#include <d3dx11async.h>
#include <fstream>
#include <time.h>
#include <math.h>

using namespace std;

class furShader
{
private:
	struct MatrixBufferType
	{
		D3DMATRIX world;
		D3DMATRIX view;
		D3DMATRIX projection;
		D3DMATRIX identity;
		D3DXMATRIX bonesTranslation[42];
		D3DXMATRIX previousTranslation[42];
		D3DXVECTOR3 camera;
		float padding;
	};

	struct FurBufferType
	{
		float length;
		D3DXVECTOR3 wind;
		D3DXMATRIX bonesTranslation[31];
		float time;
		D3DXVECTOR3 filler;
	};
	
	struct AnimationBufferType
	{
		D3DMATRIX bonestT[31];
	};

	struct ModifiedBufferType
	{
		float gravityStrength;
		float stiffnessAmp;
		float dampeningAmp;
		float modpadding;
	};

	struct CameraBufferType
	{
		D3DXVECTOR3 cameraPos;
		float reflectionMod;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
		float specularPower;
		D3DXVECTOR4 specularColor;
	};

	float diff, counter;
	float randomTarget1, randomTarget2, randomTarget3;
	float random1,random2,random3; 
	float movement1,movement2,movement3; 

public:
	furShader(void);
	~furShader(void);

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, D3DXMATRIX*, D3DXMATRIX*, int, D3DXVECTOR3, D3DXVECTOR3, D3DXVECTOR3, float, float, float, float, float);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutDownShader();
	void OutputShaderError(ID3D10Blob*, HWND, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, D3DXMATRIX*, D3DXMATRIX*, int, D3DXVECTOR3, D3DXVECTOR3, D3DXVECTOR3, float, float, float, float, float);
	void RenderShader(ID3D11DeviceContext*, int);
	void ShutdownShader();

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_samplerState;
	ID3D11SamplerState* m_samplerState1;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_furBuffer;
	ID3D11Buffer* m_animBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_modBuffer;
};

#endif