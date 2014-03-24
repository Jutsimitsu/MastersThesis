#pragma once

#define MAX_VERTICES 270000
#define MAX_POLIGONS 270000

#include <math.h>
#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>
#include <fbxsdk\core\base\fbxtime.h>
#include <D3DX11.h>
#include <D3DX10.h>
#include <D3DX10math.h>
#include <list>
#include <hash_map>
#include <limits>
#include "BoneData.h"
#include "SkeletalData.h"
#include "AnimationData.h"
#include "AnimationFrame.h"

using namespace std;

class fbxLoader
{
private:
	float testpositioning;


	FbxManager* loaderManager;
	FbxIOSettings* ios;
	FbxImporter* lImporter;
	FbxScene* scene;
	FbxMesh* lMesh;
	FbxNode* lRootNode;
	FbxNode* lNode;
	bool lImportStatus;

	int vertexMaxCount;
	int bonesMaxCount;
	int indicesMaxCount;
	int indicesMeshCount;
	int actualVertexCount;
	int actualIndicesCount;
	int counter, counter2;

	FbxSkeleton *fbxSkeleton;
	SkeletalData *skeleton;
	bool rootbone;
	bool rootbone1;
	bool cycle;

	AnimationData* animationStructure;

	D3DXVECTOR3 vertex[MAX_VERTICES];
	D3DXVECTOR4 color[MAX_VERTICES];  
    D3DXVECTOR3 normal[MAX_VERTICES];  
    D3DXVECTOR3 tangent[MAX_VERTICES];  
    D3DXVECTOR2 uv[MAX_VERTICES];  

	struct indexesStructure
	{
		unsigned int indice;
		D3DXVECTOR3 normal1;
		D3DXVECTOR2 uv1;
		//D3DXVECTOR4 color1;
	};

	struct weightsStructure
	{
		int boneID;
		double weight;
		weightsStructure() : boneID (0), weight(0){}
		weightsStructure( int bonei, int num, double weighti  ) : boneID( bonei ), weight( weighti ) {}
	} ;

	struct vertexWeights
	{
		D3DXVECTOR3 position;
		weightsStructure weights[4];
		int weightsNum;
	};

	list<D3DXVECTOR3*> vertexLists;
	list<indexesStructure*> indiceLists;
	
	weightsStructure** weights;
	int weightsNumber;
		
	vertexWeights* vertexArray;
	FbxVector4 scaleBones;

	indexesStructure* indices;

	D3DXVECTOR3 maxB, maxV;

public:
	fbxLoader(void);
	fbxLoader(char*);
	~fbxLoader(void);

	SkeletalData* getSkeleton() { return skeleton; } 
	vertexWeights getVertices() { return *vertexArray; }
	D3DXVECTOR3 getVertex(int i) { return vertexArray[i].position; }
	float getWeight(int i, int j) { return vertexArray[i].weights[j].weight; }
	int getBoneWeight(int i, int j) { return vertexArray[i].weights[j].boneID; }
	int getWeightNumber(int i) { return vertexArray[i].weightsNum; }
	indexesStructure getIndices(int i) { return indices[i]; }
	//weightsStructure* getWeights(int i) { return weights[i]; }
	//int getWeightsNumber() { return weightsNumber; }
	unsigned int getIndex(int i) { return indices[i].indice; }
	int getVertexCount() { return vertexMaxCount; }
	int getIndexMaxCount() { return indicesMaxCount; }
	AnimationData* getAnimation() { return animationStructure; }
	D3DXVECTOR4 getScale()
	{
		return D3DXVECTOR4 (scaleBones.mData[0], scaleBones.mData[1], scaleBones.mData[2], scaleBones.mData[3]);
	}
	D3DXVECTOR3 getSc() { return D3DXVECTOR3(maxB.x/maxV.x, maxB.y/maxV.y, maxB.z/maxV.z); }
	void Shutdown();

private:
	void loadFbx(char*);

	void processNode(FbxNode*);
	void processMesh(FbxNode*);
	void readVertex(FbxMesh*, int, D3DXVECTOR3*);
	void readColor(FbxMesh*, int, int, D3DXVECTOR4*);
	void readUV(FbxMesh*, int, int, D3DXVECTOR2*);
	void readNormal(FbxMesh*, int, D3DXVECTOR3*);
	void readTangent(FbxMesh*, int, int, D3DXVECTOR3*);
	void readIndex(FbxMesh*, int, int, int*);
	void readAnimation(FbxMesh*, int, int);
	void readSkin(FbxNode*);
	void readSkeleton(FbxNode*); 
	void buildSkeletonCycle(FbxNode*, int);
	void buildSkeleton(FbxNode*);
	FbxNode* findSkeletonRootBone(FbxNode*);
	bool IsMeshSkeleton(FbxNode*);
	void setBindPose(FbxNode*);
	void setBindPoseCluster(FbxNode*);
	void buildAnimation(FbxSkeleton*);
	void ComputeDualQuaternionDeformation(FbxMesh*, FbxAMatrix &, const FbxTime&, int);
	void ComputeClusterDeformation(FbxMesh*, FbxAMatrix&, FbxCluster*, FbxAMatrix&, const FbxTime&, FbxPose*);
	void readAnimationWeigths(FbxMesh*);
	void readAnimationTakes(FbxNode*);
	void readAnimationTakeData(FbxNode*);
	void convertScale();
	FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose = NULL, FbxAMatrix* pParentGlobalPosition = NULL);
	FbxAMatrix fbxLoader::GetPoseMatrix(FbxPose*, int);
	FbxAMatrix fbxLoader::GetGeometry(FbxNode*);

	D3DXVECTOR4 getRotationVector(FbxMatrix matrix);
	float doubleToFloat(double);
};
