#include "textureProcessor.h"


textureProcessor::textureProcessor(void)
{
	texture = 0;
}


textureProcessor::~textureProcessor(void)
{
}

bool textureProcessor::InitializeGPU(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;

	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &texture, NULL);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void textureProcessor::Shutdown()
{
	if(texture)
	{
		texture->Release();
		texture=0;
	}
}

ID3D11ShaderResourceView* textureProcessor::GetTexture()
{
	return texture;
}
