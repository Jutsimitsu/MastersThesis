#pragma once

#ifndef _TEXTUREPROCESSOR_H_
#define _TEXTUREPROCESSOR_H_

#include <d3d11.h>
#include <D3DX11tex.h>
#include <Wincodecsdk.h>

class textureProcessor
{
public:
	textureProcessor(void);
	~textureProcessor(void);

	bool InitializeGPU(ID3D11Device*, WCHAR*);
	void Shutdown();
	
	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* texture;
};

#endif