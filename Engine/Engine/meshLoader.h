#ifndef _MeshLoader_h_
#define _MeshLoader_h_

#pragma once
#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <fbxsdk.h>
using namespace std;

typedef struct
{
	float x, y, z;
}VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
}FaceType;

class meshLoader
{
private: 
	VertexType *vertices, *texcoords, *normals;
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	FaceType *faces;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

public:
	meshLoader(void);
	//meshLoader(const meshLoader&);
	meshLoader(char*);
	meshLoader(char*, int, int, int, int);
	~meshLoader(void);
	void Render(ID3D11DeviceContext*);

private:
	void LoadModel(char*);
	void GetModelFilename(char*);
	bool ReadFileCounts(char*, int&, int&, int&, int&);
	bool LoadDataStructures(char*, int, int, int, int);
	void ReleaseData(void);
	void RenderBuffers(ID3D11DeviceContext*, int, int);
	bool InitializeBuffers(ID3D11Device*, int, int);
	void ShutdownBuffers();
};

#endif
