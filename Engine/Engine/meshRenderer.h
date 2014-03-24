#pragma once

#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>
#include <D3DX11.h>
#include <D3DX10math.h>

#include "fbxLoader2.h"
#include "textureProcessor.h"

class meshRenderer
{
public:
	meshRenderer(void);
	~meshRenderer(void);
	meshRenderer(fbxLoader2 &m_loaderm);
	void RenderScene(ID3D11DeviceContext *context);
	bool Initialize(ID3D11Device *device, WCHAR*, D3DXVECTOR4**, int, int);
	void ShutDown();
	ID3D11ShaderResourceView* GetTexture();

private: 	
	struct vertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		int bones[4];
		float weights[4];
		float hairLength;
		D3DXVECTOR2 uv;
	};

	vertexType *vertices;

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	fbxLoader2 loader;
	textureProcessor* tex;

	bool InitializeBuffers(ID3D11Device*, D3DXVECTOR4**, int, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();
};

