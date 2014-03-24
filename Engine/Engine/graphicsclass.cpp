#include "graphicsclass.h"



GraphicsClass::GraphicsClass()
{
	m_DX = 0;
	cam = 0;
	loader = 0;
	renderer = 0;
	rotation = 0.0f;
	m_ColorShader = 0;
	m_FurShader = 0;
	bmpProcessor = 0;
	furGenerator = 0;
	numLayers = 0;
	step = 0;
	animationcycle = 0;
	changeRotation = true;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}

GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	numLayers = 35;
	generatedLayers = 4;
	animationcycle = 0;
	lightcycle = 0;
	changeRotation = true;
	changeAnimation = true;
	
	wind = 1;
	gravity = 1; 
	stiffness = 1;
	dampening = 1;
	reflect = 1;
	hairGaps = 0.5f;

	m_DX = new DirectManager;

	if(!m_DX)
	{
		return false;
	}

	result = m_DX -> Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd,L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	cam = new camera;
	if(!cam)
	{
		return false;
	}

	m_ColorShader = new phongShader;
	if(!m_ColorShader)
	{
		return false;
	}

	m_FurShader = new furShader;
	if(!m_FurShader)
	{
		return false;
	}

	bmpProcessor = new bitmapProcessor;
	if(!bmpProcessor)
	{
		return false;
	}

	furGenerator = new furDensityGenerator;
	if(!furGenerator)
	{
		return false;
	}

	result = m_ColorShader->Initialize(m_DX->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	result = m_FurShader->Initialize(m_DX->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the fur shader object.", L"Error", MB_OK);
		return false;
	}

	bmpProcessor->readBMP("C:\\wolftexwithalpha1.bmp");
	furGenerator->Initialize(m_DX->GetDevice(), m_DX->GetDeviceContext(), 383832, 2048, generatedLayers, 1550000, bmpProcessor->getBitmap(), bmpProcessor->getWidth(), bmpProcessor->getHeight());

	loader = new fbxLoader2("C:\\wilkanimowany4.FBX");
	renderer = new meshRenderer(*loader);
	result = renderer->Initialize(m_DX->GetDevice(), L"C:\\wolftexwithalpha1.bmp", bmpProcessor->getBitmap(), bmpProcessor->getWidth(), bmpProcessor->getHeight());

	bmpProcessor->drawImage(bmpProcessor->getBitmap());

	if (!result)
	{
		return false;
	}

	rotation = 180.0f;

	cam->SetPosition(0.0f, -14.5f, 6.0f);

	return true;
}

void GraphicsClass::Shutdown()
{
	if(m_DX)
	{
		m_DX ->Shutdown();
		delete m_DX;
		m_DX = 0;
	}
	if(loader)
	{
		loader->Shutdown();
		delete loader;
		loader = 0;
	}
	if(renderer)
	{
		renderer->ShutDown();
		delete renderer;
		renderer = 0;
	}
	if(cam)
	{
		cam->Shutdown();
		delete cam;
		cam = 0;
	}
	if(m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}
	if(m_FurShader)
	{
		m_FurShader->Shutdown();
		delete m_FurShader;
		m_FurShader = 0;
	}
	if(furGenerator)
	{
		furGenerator->Shutdown();
		delete furGenerator;
		furGenerator = 0;
	}
	if(bmpProcessor)
	{
		bmpProcessor->Shutdown();
		delete bmpProcessor;
		bmpProcessor = 0;
	}
	
	return;
}

bool GraphicsClass::Frame()
{
	bool result;

	result = Render();
	if(!result)
	{
		result = false;
	}

	return true;
}

bool GraphicsClass::Render()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3DXMATRIX rotatex, rotatey, rotatez;
	D3DXMatrixIdentity(&rotatex);
	D3DXMatrixIdentity(&rotatey);
	D3DXMatrixIdentity(&rotatez);
	bool result;

	if(changeRotation == true)
	{
		rotation += (float)D3DX_PI * 0.001f;
	}
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	if(changeAnimation == true)
	{
		step++;
		if(step%1 == 0)
		{
			animationcycle += 1;
			step = 0;
		}
	}
	if (animationcycle >= loader->getAnimation()->GetFramesNumber())
	{
		animationcycle = 2;
	}

	lightcycle += PI/180;
	if (lightcycle >= 2*PI)
	{
		lightcycle = 0;
	}

	m_DX->BeginScene(0.5f, 0.5f, 1.0f, 0.0f);

	// Generate the view matrix based on the camera's position.
	cam->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	cam->GetViewMatrix(viewMatrix);
	m_DX->GetWorldMatrix(worldMatrix);
	m_DX->GetProjectionMatrix(projectionMatrix);
	
	D3DXMatrixRotationZ(&rotatey, D3DX_PI/8+rotation);
	D3DXMatrixRotationY(&rotatex, D3DX_PI/4);

	D3DXMATRIX *bonematrices =  new D3DXMATRIX[loader->getSkeleton()->GetBonesCount()];
	D3DXMATRIX *previousmatrices =  new D3DXMATRIX[loader->getSkeleton()->GetBonesCount()];

	for(int j = 0; j<loader->getSkeleton()->GetBonesCount(); j++)
	{
		bonematrices[j] = loader->getAnimation()->GetSkeleton(animationcycle)->GetBone(loader->getSkeleton()->GetBoneIndex(j))->GetTransformation();//sprawdzic getindex bone, chyba indeksy sa zle
		//bonematrices[j] = loader->getSkeleton()->GetBone(loader->getSkeleton()->GetBoneIndex(j))->GetTransformation();

		if (animationcycle > 2)
		{
			previousmatrices[j] = loader->getAnimation()->GetSkeleton(animationcycle-1)->GetBone(loader->getSkeleton()->GetBoneIndex(j))->GetTransformation();
		}
		else 
		{
			previousmatrices[j] = loader->getAnimation()->GetSkeleton(loader->getAnimation()->GetFramesNumber()-1)->GetBone(loader->getSkeleton()->GetBoneIndex(j))->GetTransformation();
		}
		continue;
	}

	renderer->RenderScene(m_DX->GetDeviceContext());

	result = m_ColorShader->Render(m_DX->GetDeviceContext(), loader->getIndexMaxCount(), worldMatrix*rotatey, viewMatrix, projectionMatrix, cam->GetPosition(), D3DXVECTOR3(2.0f, 5.0f, -0.2f) , D3DXVECTOR4(0.1f, 0.1f, 0.1f, 0.1f), D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), D3DXVECTOR4 (1.0f, 1.0f, 1.0f, 1.0f), 32.0f, renderer->GetTexture(), bonematrices, loader->getSkeleton()->GetBonesCount());
	if(!result)
	{
		return false;
	}

	m_DX->TurnOnAlphaBlending();

	for(int i=0; i<numLayers; i++)
	{
		float layer = float(i) / numLayers;
		float length = hairGaps * layer;
		int usedLayer = generatedLayers-(int)floor(((float)generatedLayers/(float)numLayers)*i)-1;

		wind;

		result = m_FurShader->Render(m_DX->GetDeviceContext(), loader->getIndexMaxCount(), worldMatrix*rotatey, viewMatrix, projectionMatrix, furGenerator->GetTexture(usedLayer),
			length, bonematrices, previousmatrices, loader->getSkeleton()->GetBonesCount(), cam->GetPosition(), D3DXVECTOR3(100*(float)cos(lightcycle),50,-100*(float)sin(lightcycle)), 
			cam->GetPosition(), wind, gravity, dampening, stiffness, reflect);
		if(!result)
		{
			return false;
		}
	}

	m_DX->TurnOffAlphaBlending();
	m_DX->EndScene();

	return true;
}


void GraphicsClass::AddWind()
{ 
	if(wind < 6.0f) 
		wind+=0.5f; 
}
void GraphicsClass::SubWind(){ if(wind > 0) wind-=0.5f; }
	
void GraphicsClass::AddGrav(){ if(gravity < 10) gravity +=0.1f; }
void GraphicsClass::SubGrav(){ if(gravity > 0) gravity -=0.1f; }

void GraphicsClass::AddDamp(){ if(dampening < 10) dampening +=0.1f; }
void GraphicsClass::SubDamp(){ if(dampening > 1) dampening -=0.1f; }

void GraphicsClass::AddStiff(){ if(stiffness < 10) stiffness +=0.1f; }
void GraphicsClass::SubStiff(){ if(stiffness > 1) stiffness -=0.1f; }

void GraphicsClass::AddRef(){ if(reflect < 10) reflect +=0.1f; }
void GraphicsClass::SubRef(){ if(reflect > 0) reflect -=0.1f; }

void GraphicsClass::AddGap(){ if(hairGaps < 2.0f) hairGaps +=0.03f; }
void GraphicsClass::SubGap(){ if(hairGaps > 0.1f) hairGaps -=0.03f; }

void GraphicsClass::ResetSettings() 
{
	wind = 1;
	dampening = 1;
	gravity = 1;
	stiffness = 1;
	reflect = 1;
}