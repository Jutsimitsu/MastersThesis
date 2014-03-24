#pragma once

#ifndef _PHONGSHADERCLASS_H_
#define _PHONGSHADERCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;

class phongShader
{
private:
	struct MatrixBufferType
	{
		D3DMATRIX world;
		D3DMATRIX view;
		D3DMATRIX projection;
		D3DXMATRIX bonesTranslation[42];
	};

	struct CameraBufferType
	{
		D3DXVECTOR3 eye;
		float padding;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
		float specularPower;
		D3DXVECTOR4 specularColor;
	};

public:
	phongShader(void);
	~phongShader(void);

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR3,
	D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4, float,ID3D11ShaderResourceView*, D3DXMATRIX*, int);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutDownShader();
	void OutputShaderError(ID3D10Blob*, HWND, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR3,
	D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4, float,ID3D11ShaderResourceView*, D3DXMATRIX*, int);
	void RenderShader(ID3D11DeviceContext*, int);
	void ShutdownShader();

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_samplerState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
};

#endif