#pragma once

#define MAX_VERTICES 270000
#define MAX_POLIGONS 270000

#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>
#include <fbxsdk\core\base\fbxtime.h>
#include <D3DX11.h>
#include <D3DX10math.h>
#include <list>
#include "AnimationData.h"
#include "BoneData.h"
#include "SkeletalData.h"


using namespace std;

class fbxLoader2
{
private:
	FbxManager* loaderManager;
	FbxIOSettings* ios;
	FbxImporter* lImporter;
	FbxScene* scene;
	FbxMesh* lMesh;
	FbxNode* lRootNode;
	FbxNode* lNode;
	bool lImportStatus;

	int vertexMaxCount;
	int indicesMaxCount;
	int indicesMeshCount;
	int actualVertexCount;
	int actualIndicesCount;
	int counter, counter2;

	D3DXVECTOR3 vertex[MAX_VERTICES];
	D3DXVECTOR4 color[MAX_VERTICES];  
    D3DXVECTOR3 normal[MAX_VERTICES];  
    D3DXVECTOR3 tangent[MAX_VERTICES];  
    D3DXVECTOR2 uv[MAX_VERTICES];  

	FbxSkeleton *fbxSkeleton;
	SkeletalData *skeleton;
	AnimationData* animationStructure;

	struct indexesStructure
	{
		unsigned int indice;
		D3DXVECTOR3 normal1;
		D3DXVECTOR2 uv1;
	};

	struct weightsStructure
	{
		int boneID;
		double weight;
		weightsStructure() : boneID (0), weight(0){}
		weightsStructure( int bonei, int num, double weighti  ) : boneID( bonei ), weight( weighti ) {}
	};

	struct vertexWeights
	{
		D3DXVECTOR3 position;
		weightsStructure weights[4];
		int weightsNum;
	};

	weightsStructure** weights;
	int weightsNumber;
	vertexWeights* vertexArray;

	list<D3DXVECTOR3*> vertexLists;
	list<indexesStructure*> indiceLists;
		
	//D3DXVECTOR3* vertexArray;
	indexesStructure* indices;

public:
	fbxLoader2(void);
	fbxLoader2(char*);
	~fbxLoader2(void);

	SkeletalData* getSkeleton() { return skeleton; } 
	vertexWeights getVertices() { return *vertexArray; }
	D3DXVECTOR3 getVertex(int i) { return vertexArray[i].position; }
	indexesStructure getIndices(int i) { return indices[i]; }
	unsigned int getIndex(int i) { return indices[i].indice; }
	int getVertexCount() { return vertexMaxCount; }
	int getIndexMaxCount() { return indicesMeshCount; }
	AnimationData* getAnimation() { return animationStructure; }
	void Shutdown();

	float getWeight(int i, int j) { return vertexArray[i].weights[j].weight; }
	int getBoneWeight(int i, int j) { return vertexArray[i].weights[j].boneID; }
	int getWeightNumber(int i) { return vertexArray[i].weightsNum; }

private:
	void loadFbx(char*);

	void processNode(FbxNode*);
	void processMesh(FbxNode*);

	void readVertexData(FbxNode*);
	void readVertex(FbxMesh*, int, D3DXVECTOR3*);
	void readColor(FbxMesh*, int, int, D3DXVECTOR4*);
	void readUV(FbxMesh*, int, int, D3DXVECTOR2*);
	void readNormal(FbxMesh*, int, D3DXVECTOR3*);
	void readTangent(FbxMesh*, int, int, D3DXVECTOR3*);
	void readIndex(FbxMesh*, int, int, int*);
	void buildAnimation(FbxNode*);
	void readAnimation(FbxMesh*, int, int);
	void readAnimationTakeData(FbxNode*);

	void buildSkeletonCycle(FbxNode*, int);
	void buildSkeleton(FbxNode*);
	FbxNode* findSkeletonRootBone(FbxNode*);
	bool IsMeshSkeleton(FbxNode*);
	void readAnimationWeigths(FbxMesh*);
	void setBindPoseCluster(FbxNode*);

	void computeMesh(FbxNode*, FbxTime&, FbxAnimLayer*, FbxAMatrix&, FbxPose*, int);
	void ComputeSkinDeformation(FbxAMatrix&, FbxMesh*, FbxTime&, FbxPose*, int);
	void ComputeDualQuaternionDeformation(FbxAMatrix&, FbxMesh*, FbxTime&, FbxPose*, int);
	void ComputeLinearDeformation(FbxAMatrix&, FbxMesh*,  FbxTime&, FbxPose*, int);
	void ComputeClusterDeformation(FbxAMatrix&, FbxMesh*, FbxCluster*,  FbxAMatrix&, FbxTime, FbxPose*, int);

	FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose = NULL, FbxAMatrix* pParentGlobalPosition = NULL);
	FbxAMatrix GetPoseMatrix(FbxPose*, int);
	FbxAMatrix GetGeometry(FbxNode*);

	void repairAnimation();

};
