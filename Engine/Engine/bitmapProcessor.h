#pragma once

#include <stdio.h>
#include <Windows.h>
#include <D3DX11.h>
#include <D3DX10math.h>
#include <string>
#include <iostream>
#include <sstream> 

class bitmapProcessor
{
private:
	int width;
	int height;

	D3DXVECTOR4** alphaChannelMap;

public:
	bitmapProcessor(void);
	~bitmapProcessor(void);

	void Shutdown();
	void readBMP(char*);
	void drawImage(D3DXVECTOR4**);

	D3DXVECTOR4** getBitmap() { return alphaChannelMap; }
	int getWidth() { return width; }
	int getHeight() { return height; }
};

