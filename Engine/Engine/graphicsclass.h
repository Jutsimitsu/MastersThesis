#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


#include <windows.h>
#include "directmanager.h"
#include "meshLoader.h"
#include "fbxLoader2.h"
#include "meshRenderer.h"
#include "camera.h"
#include "phongShader.h"
#include "textureProcessor.h"
#include "furDensityGenerator.h"
#include "furShader.h"
#include "bitmapProcessor.h"
#include <time.h> 
#include <math.h>

#define PI 3.14159265

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

	void SetCam(bool rot){ changeRotation = rot; }
	bool GetCam(){ return changeRotation; }
	void SetAnim(bool anim){ changeAnimation = anim; }
	bool GetAnim(){ return changeAnimation; }
		
	void AddWind();
	void SubWind();

	void AddGrav();
	void SubGrav();

	void AddDamp();
	void SubDamp();

	void AddStiff();
	void SubStiff();

	void AddRef();
	void SubRef();

	void AddGap();
	void SubGap();

	void ResetSettings();

private:
	bool Render();

private:
	float rotation;
	int numLayers;
	int generatedLayers;

	DirectManager* m_DX;
	fbxLoader2* loader;
	camera *cam;
	meshRenderer* renderer;
	phongShader* m_ColorShader;
	furShader *m_FurShader;
	bitmapProcessor *bmpProcessor;
	furDensityGenerator* furGenerator;
	int animationcycle; 
	float lightcycle;
	int step;

	float wind;
	float hairLength;
	float gravity;
	float dampening;
	float stiffness;
	float reflect;
	float hairGaps;

	bool changeRotation;
	bool changeAnimation;
};

#endif