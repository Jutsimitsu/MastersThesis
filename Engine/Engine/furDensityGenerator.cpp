#include "furDensityGenerator.h"


furDensityGenerator::furDensityGenerator(void)
{
	m_size = 0;
	m_layers = 0;
	width = 0;
	height = 0;
}


furDensityGenerator::~furDensityGenerator(void)
{
}

bool furDensityGenerator::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, unsigned int seed, unsigned int size, unsigned int number, unsigned int idensity, D3DXVECTOR4** alphaMap, int alphaX, int alphaY)
{
	srand(seed);
	m_size = size;
	m_layers = number;
	width = alphaX;
	height = alphaY;
	int thickness = 2;
	textures = new ID3D11ShaderResourceView*[number];
	int tempWidth = width;
	double xu;
	double yv;
	int xvalue;
	int yvalue;
				
	float hairLength;
	float alfaValue;

	D3DXVECTOR4 *colorData = new D3DXVECTOR4[m_layers*m_size*m_size];

	for(int i = 0; i<m_layers; i++)
	{
		for(int j = 0; j < m_size; j++)
		{
			for(int k = 0; k < m_size; k++)
			{
				colorData[m_size*m_size*(i) + m_size*(j) + (k)] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}
			
	hairLength = 1.0f/(float)m_layers;
	hairLength = 1-hairLength;
	int density = idensity*hairLength*thickness;
	srand(30000);

	for (int j = 0; j<density;j++)
	{
		int randomX = rnd(0,m_size-1);
		int randomY = rnd(0,m_size-1);

		
		xu = (double)randomX/(double)m_size;
		yv = (double)randomY/(double)m_size;
		xvalue = (int)floor((double)width*xu);
		yvalue = (int)ceil((double)height*yv);

		if(yvalue > height) 
			yvalue = height;
		if(yvalue < 0)
			yvalue = 0;

		colorData[m_size*m_size*(0) + m_size*(randomY) + (randomX)] = D3DXVECTOR4(alphaMap[yvalue][xvalue].x,alphaMap[yvalue][xvalue].y,alphaMap[yvalue][xvalue].z,255);
	}


	//copy layer 0 to all layers
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			for(int k=1; k<m_layers; k++)
			{
				colorData[m_size*m_size*(k) + m_size*(j) + (i)].x = colorData[m_size*m_size*(0) + m_size*(j) + (i)].x;
				colorData[m_size*m_size*(k) + m_size*(j) + (i)].y = colorData[m_size*m_size*(0) + m_size*(j) + (i)].y;
				colorData[m_size*m_size*(k) + m_size*(j) + (i)].z = colorData[m_size*m_size*(0) + m_size*(j) + (i)].z;
				colorData[m_size*m_size*(k) + m_size*(j) + (i)].w = colorData[m_size*m_size*(0) + m_size*(j) + (i)].w;
			}
		}
	}

	//blur layers
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			for(int k=0; k<m_layers; k++)
			{
				hairLength = (float)k/m_layers;
				float alfa = colorData[m_size*m_size*(k) + m_size*(j) + (i)].w;

				xu = (double)i/(double)m_size;
				yv = (double)j/(double)m_size;
				xvalue = (int)floor((double)width*xu);
				yvalue = (int)ceil((double)height*yv);

				float layers_in_point = ceil((1.0f-(alphaMap[yvalue][xvalue].w/255.0f)) * m_layers);

				if(alfa>0.0f)
				{
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].w = hairLength;
				}
			}
		}
	}


	//adjust color of hair on different layers
	/*for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			for(int k=0; k<m_layers; k++)
			{
				hairLength = (float)k/m_layers; // 0 to 1
				float scale = 1-hairLength;

				scale = max(scale, 0.94);
				float alpha = colorData[m_size*m_size*(k) + m_size*(j) + (i)].w;
				if( alpha > 0.0f )
				{
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].x *= scale;
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].y *= scale;
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].z *= scale;
				}
			}
		}
	}*/


	//adjust layers with alpha value

	//remove hair from 0 layers.
	for(int k=0; k<m_layers; k++)
	{
		for (int i = 0; i < m_size; i++)
		{
			for (int j = 0; j < m_size; j++)
			{
				xu = (double)j/(double)m_size;
				yv = (double)i/(double)m_size;
				xvalue = (int)floor((double)width*xu);
				yvalue = (int)ceil((double)height*yv);

				if(yvalue > height) 
					yvalue = height;
				if(yvalue < 0)
					yvalue = 0;

				if(alphaMap[yvalue][xvalue].w > 250.0f)
				{
					colorData[m_size*m_size*(k) + m_size*(i) + (j)] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f); // removes hair where there are none
				}
			}
		}
	}

	union color
	{
		unsigned int value;
		unsigned char component[4];
	};
	
	const WCHAR filenames[15][20] = {{L"C:\\wolfgen1.png"}, {L"C:\\wolfgen2.png"},{L"C:\\wolfgen3.png"},{L"C:\\wolfgen4.png"},{L"C:\\wolfgen5.png"},{L"C:\\wolfgen6.png"},{L"C:\\wolfgen7.png"},{L"C:\\wolfgen8.png"},{L"C:\\wolfgen9.png"},
	{L"C:\\wolfgen10.png"},{L"C:\\wolfgen11.png"},{L"C:\\wolfgen12.png"},{L"C:\\wolfgen13.png"},{L"C:\\wolfgen14.png"},{L"C:\\wolfgen15.png"}};

	//create textures
	for (int i = 0; i<m_layers; i++)
	{
		DWORD *pixels = new DWORD[m_size*m_size];

		for(int j = 0; j<m_size; j++)
		{
			for(int k = 0; k<m_size; k++)
			{					
				BYTE r = colorData[m_size*m_size*(i) + m_size*(j) + (k)].x;// * 255;
				BYTE g = colorData[m_size*m_size*(i) + m_size*(j) + (k)].y;// * 255;
				BYTE b = colorData[m_size*m_size*(i) + m_size*(j) + (k)].z;// * 255;
				BYTE a = colorData[m_size*m_size*(i) + m_size*(j) + (k)].w * 255;

				pixels[j*m_size+k] = (a << 24) | (b << 16) | (g << 8) | r; // konwersja na dword... bo tak.
			}
		}

		ID3D11Texture2D *mappedTex = 0;
		D3D11_TEXTURE2D_DESC desc;

		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = m_size;
		desc.Width = m_size;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA texData;
		texData.pSysMem = pixels;
		texData.SysMemPitch = (UINT) (m_size*4);
		texData.SysMemSlicePitch = (UINT) (m_size * m_size *4);

		device->CreateTexture2D(&desc, &texData, &mappedTex);

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = desc.Format;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;

		ID3D11RenderTargetView *pRenderTargetView = NULL;
		device->CreateRenderTargetView( mappedTex, &rtDesc, &pRenderTargetView );

		D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
		srDesc.Format = desc.Format;
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srDesc.Texture2D.MostDetailedMip = 0;
		srDesc.Texture2D.MipLevels = 1;

		ID3D11ShaderResourceView *pShaderResView = NULL;
		device->CreateShaderResourceView( mappedTex, &srDesc, &pShaderResView );
				
		textures[i] = pShaderResView;

		//D3DX11SaveTextureToFile(context, mappedTex, D3DX11_IFF_PNG, filenames[i]);

		delete[] pixels;
		pixels = 0;
	}

	//this->drawImage(alphaChannelMap);

	delete[] colorData;
	colorData = 0;

	return true;
}


ID3D11ShaderResourceView* furDensityGenerator::GetTexture(int layer)
{
	//int temp = (int)floor(m_layers * max(0.0, min(0.9999, layer)));

	return textures[layer];
}

void furDensityGenerator::Shutdown()
{
	m_size = 0;
	m_layers=0;
}



/*bool furDensityGenerator::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, unsigned int seed, unsigned int size, unsigned int number, unsigned int idensity, char* name)
{
	srand(seed);
	m_size = size;
	m_layers = number;
	int thickness = 2;
	textures = new ID3D11ShaderResourceView*[number];

	D3DXVECTOR4 *colorData = new D3DXVECTOR4[m_layers*m_size*m_size];

	for(int i = 0; i<m_layers; i++)
	{
		for(int j = 0; j < m_size; j++)
		{
			for(int k = 0; k < m_size; k++)
			{
				colorData[m_size*m_size*(i) + m_size*(j) + (k)] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, (unsigned) 0);
			}
		}
	}
			
	// generate hair layers
	for(int i = 0; i<m_layers; i++)
	{
		float hairLength = (float)i/m_layers;
		hairLength = 1-hairLength;
		int density = idensity*hairLength*thickness;
		srand(30000);

		for (int j = 0; j<density;j++)
		{
			int randomX = rnd(0,m_size-1);
			int randomY = rnd(0,m_size-1);

			for (int l = 0; l<4; l++)
				colorData[m_size*m_size*(i) + m_size*(randomY) + (randomX)] = D3DXVECTOR4(0.5f,0.5f,0.5f, 1.0f);
		}
	}

	//blur layers
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			for(int k=0; k<m_layers; k++)
			{
				float hairLength = (float)k/m_layers;
				float alfa = colorData[m_size*m_size*(k) + m_size*(j) + (i)].w;

				if(alfa>0.0f)
				{
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].w = 1-hairLength;
				}
			}
		}
	}

	//adjust color of hair on different layers
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			for(int k=0; k<m_layers; k++)
			{
				float hairLength = (float)k/m_layers; // 0 to 1
				float scale = 1-hairLength;

				scale = max(scale, 0.9);
				float alpha = colorData[m_size*m_size*(k) + m_size*(j) + (i)].w;
				if( alpha > 0.0f )
				{
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].x *= scale;
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].y *= scale;
					colorData[m_size*m_size*(k) + m_size*(j) + (i)].z *= scale;
				}
			}
		}
	}

	union color
	{
		unsigned int value;
		unsigned char component[4];
	};

	//create textures
	for (int i = 0; i<m_layers; i++)
	{
		DWORD *pixels = new DWORD[m_size*m_size];

		for(int j = 0; j<m_size; j++)
		{
			for(int k = 0; k<m_size; k++)
			{	
				BYTE r = colorData[m_size*m_size*(i) + m_size*(j) + (k)].x * 255;
				BYTE g = colorData[m_size*m_size*(i) + m_size*(j) + (k)].y * 255;
				BYTE b = colorData[m_size*m_size*(i) + m_size*(j) + (k)].z * 255;
				BYTE a = colorData[m_size*m_size*(i) + m_size*(j) + (k)].w * 255;
				pixels[j*m_size+k] = (a << 24) | (b << 16) | (g << 8) | r; // konwersja na dword... bo tak.
			}
		}

		ID3D11Texture2D *mappedTex = 0;
		D3D11_TEXTURE2D_DESC desc;

		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = m_size;
		desc.Width = m_size;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA texData;
		texData.pSysMem = pixels;
		texData.SysMemPitch = (UINT) (m_size*4);
		texData.SysMemSlicePitch = (UINT) (m_size * m_size *4);

		device->CreateTexture2D(&desc, &texData, &mappedTex);

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = desc.Format;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;

		ID3D11RenderTargetView *pRenderTargetView = NULL;
		device->CreateRenderTargetView( mappedTex, &rtDesc, &pRenderTargetView );

		D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
		srDesc.Format = desc.Format;
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srDesc.Texture2D.MostDetailedMip = 0;
		srDesc.Texture2D.MipLevels = 1;

		ID3D11ShaderResourceView *pShaderResView = NULL;
		device->CreateShaderResourceView( mappedTex, &srDesc, &pShaderResView );

		textures[i] = pShaderResView;

		delete[] pixels;

		D3DX11SaveTextureToFile(context, mappedTex, D3DX11_IFF_BMP, L"C:\\generatedFur.png");

	}

	delete[] colorData;

	return true;
}
*/