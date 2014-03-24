#pragma once

#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10math.h>
#include <string>
#include <iostream>
#include <sstream>

#define MAX_LAYERS 50

const float INV_RAND_MAX = 1.0 / (RAND_MAX + 1);
inline float rnd(float max=1.0) { return max * INV_RAND_MAX * rand(); }
inline float rnd(float min, float max) { return min + (max - min) * INV_RAND_MAX * rand(); }


class furDensityGenerator
{
public:
	furDensityGenerator(void);
	~furDensityGenerator(void);

	bool Initialize(ID3D11Device *, ID3D11DeviceContext*, unsigned int, unsigned int, unsigned int, unsigned int, D3DXVECTOR4**, int, int);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture(int);

private:
	unsigned int m_size;
	unsigned int m_layers;
	int width;
	int height;

	ID3D11ShaderResourceView **textures;

	ID3D11Texture2D* m_textures[MAX_LAYERS];
};

