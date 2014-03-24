#include "fbxLoader.h"


fbxLoader::fbxLoader(void)
{
}


fbxLoader::~fbxLoader(void)
{
}


fbxLoader::fbxLoader(char* filename)
{
	testpositioning = 0.0f;


	loaderManager = 0;
	ios = 0;
	lImporter = 0;
	lImportStatus = 0;
	
	vertexMaxCount = 0;
	indicesMaxCount = 0;
	actualVertexCount = 0;
	actualIndicesCount = 0;
	indicesMeshCount = 0;
	bonesMaxCount = 0;

	fbxSkeleton = 0;
	skeleton = 0;
	//weights = 0;
	rootbone = false;
	rootbone1 = false;
	cycle = false;

	animationStructure = 0;

	maxV = D3DXVECTOR3(0,0,0);
	maxB = D3DXVECTOR3(0,0,0);

	loadFbx(filename);
}

void fbxLoader::Shutdown()
{
	loaderManager = 0;
	ios = 0;
	lImporter = 0;
	lImportStatus = 0;
	
	vertexMaxCount = 0;
	indicesMaxCount = 0;
	actualVertexCount = 0;
	actualIndicesCount = 0;
	indicesMeshCount = 0;
		
	if (indices)
		delete[] indices;

	if (vertexArray)
		delete[] vertexArray;

}

void fbxLoader::loadFbx(char* filename)
{
	//create memory manager
	loaderManager = FbxManager::Create();
	//create settings for input/output
	ios = FbxIOSettings::Create(loaderManager, IOSROOT);
	loaderManager->SetIOSettings(ios);

	lImporter = FbxImporter::Create(loaderManager, "");
	lImportStatus = lImporter->Initialize(filename, -1, loaderManager->GetIOSettings());

	scene = FbxScene::Create(loaderManager, "newscene");
    FbxAxisSystem SceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
    FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
    if( SceneAxisSystem != OurAxisSystem )
    {
        OurAxisSystem.ConvertScene(scene);
    }

    // Convert Unit System to what is used in this example, if needed
    FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
    if( SceneSystemUnit.GetScaleFactor() != 1.0 )
    {
        //The unit in this example is centimeter.
        FbxSystemUnit::cm.ConvertScene(scene);
    }

	lImporter->Import(scene);

	lRootNode = scene->GetRootNode();

	skeleton = new SkeletalData();
	animationStructure;

	//buildSkeleton(findSkeletonRootBone(lRootNode));
	//setBindPoseCluster(findSkeletonRootBone(lRootNode));
	processNode(lRootNode);

	indices = new indexesStructure[indicesMaxCount];
	vertexArray = new vertexWeights[vertexMaxCount];

	processMesh(lRootNode);
	//readSkin(lRootNode);
	//readSkeleton(lRootNode);
	//this->convertScale();
	lImporter->Destroy();

	/*for (int i = 0; i< numberOfChildren; i++)
	{
		lNode = lRootNode->GetChild(i);
		printf("Child name: %s\n", lNode->GetName());
		lMesh = lNode->GetMesh();
		if(lMesh == NULL)
		{
			printf("No mesh here\n");
		}
		else
		{
			if(lMesh->IsTriangleMesh())
			{
				int numVerts = lMesh->GetControlPointsCount();

				for (int j = 0; j < numVerts; j++)
				{
					FbxVector4 vert = lMesh->GetControlPointAt(j);
					Vertices[numberVertices].x = (float)vert.mData[0];
					Vertices[numberVertices].y = (float)vert.mData[1];
					Vertices[numberVertices].z = (float)vert.mData[2];
					numberVertices++;
				}
				int *indices = lMesh->GetPolygonVertices();
				numIndices+= lMesh->GetPolygonVertexCount();
			}
			else 
			{
				printf("Mesh not triangulated\n");
			}
			
		}
	}*/
}

void fbxLoader::processNode(FbxNode* node)
{
	//FbxNodeAttribute::EType attributeType;

	std::string s2("Nub");
	std::string s3("Footsteps");
	std::string s1(node->GetName());
	int found = s1.find(s2);
	int found2 = s1.find(s3);
	
	if(node->GetNodeAttribute())
	{
		switch(node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			vertexMaxCount += node->GetMesh()->GetControlPointsCount();
			indicesMaxCount += node->GetMesh()->GetPolygonVertexCount();
			break;

		case FbxNodeAttribute::eSkeleton:
			if (found == std::string::npos && found2 == std::string::npos)
			{
				if(this->findSkeletonRootBone(node) && rootbone == false) 
				{
					fbxSkeleton = node->GetSkeleton();
					buildSkeleton(node);
					this->readAnimationTakeData(node);
					this->buildAnimation(fbxSkeleton);
					rootbone = true;
				}
				if (found == std::string::npos && found2 == std::string::npos)
				{
					setBindPose(node);
				}
			}
			break;

		case FbxNodeAttribute::eLight:
			break;
			
		case FbxNodeAttribute::eCamera:
			break;
		}
	}

	for(int i = 0; i<node->GetChildCount(); i++)
	{
		processNode(node->GetChild(i));
	}
}

void fbxLoader::processMesh(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();
	if(!mesh)
	{
		for(int i = 0; i<node->GetChildCount(); i++)
		{
			processMesh(node->GetChild(i));
		}
		return;
	}

	if(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin)!=NULL)
	{
		this->readAnimationWeigths(mesh);
	}

	/*for(int i = 0; i<animationStructure->GetFramesNumber(); i++)
	{
		const FbxTime frameT = FbxTime(animationStructure->getDeltaTime()*i + animationStructure->getStartTime());
		FbxTime timeT = FbxTime(animationStructure->getDeltaTime()*i + animationStructure->getStartTime());
		
		FbxAMatrix pParentGlobalPosition;
		FbxAMatrix lGlobalPosition = GetGlobalPosition(node, timeT, scene->GetPose(0), &pParentGlobalPosition);
		FbxAMatrix lGeometryOffset = GetGeometry(node);
		FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

		//this->ComputeDualQuaternionDeformation(mesh, lGlobalOffPosition, frameT, i);
	}*/

	FbxSkin * lSkinDeformer = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if(mesh!=NULL && mesh->IsTriangleMesh())
	{
		for (int i = 0; i<mesh->GetControlPointsCount(); i++)
		{
			readVertex(mesh, i, &vertex[i]);
			vertexArray[i].position = D3DXVECTOR3(vertex[i]);
		}

		for (int i = 0; i<mesh->GetPolygonVertexCount(); i++)
		{
			readUV(mesh, i, 0, &uv[i]);
			readNormal(mesh, i, &normal[i]);

			indices[i].indice = mesh->GetPolygonVertices()[i];
			indices[i].normal1 = normal[i];
			indices[i].uv1 = uv[i];

			indicesMeshCount++;
		}
	}

	for(int i = 0; i<node->GetChildCount(); i++)
	{
		processMesh(node->GetChild(i)); //po prawie pół roku stwierdzam że zrobiłem to rekurencyjnie D:
	}
}

void fbxLoader::readVertex(FbxMesh* mesh, int controlPointIndex, D3DXVECTOR3* vertex)
{
	FbxVector4* ctrlPoint = mesh->GetControlPoints();

	vertex->x = (float)ctrlPoint[controlPointIndex].mData[0];
	vertex->y = (float)ctrlPoint[controlPointIndex].mData[1];
	vertex->z = (float)ctrlPoint[controlPointIndex].mData[2];

	if((float)ctrlPoint[controlPointIndex].mData[0]>maxV.x)
	{
		maxV.x = (float)ctrlPoint[controlPointIndex].mData[0];
	}

	if((float)ctrlPoint[controlPointIndex].mData[1]>maxV.y)
	{
		maxV.y = (float)ctrlPoint[controlPointIndex].mData[1];
	}

	if((float)ctrlPoint[controlPointIndex].mData[2]>maxV.z)
	{
		maxV.z = (float)ctrlPoint[controlPointIndex].mData[2];
	}
}

void fbxLoader::readColor(FbxMesh* mesh, int controlPointIndex, int vertexCounter, D3DXVECTOR4* color)
{
	if(mesh->GetElementVertexColorCount()<1)
	{
		return;
	}

	FbxGeometryElementVertexColor* vertexColor = mesh->GetElementVertexColor(0);
	switch(vertexColor->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		{
			switch(vertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					color->x = (float)vertexColor->GetDirectArray().GetAt(controlPointIndex).mRed;
					color->y = (float)vertexColor->GetDirectArray().GetAt(controlPointIndex).mGreen;
					color->z = (float)vertexColor->GetDirectArray().GetAt(controlPointIndex).mBlue;
					color->w = (float)vertexColor->GetDirectArray().GetAt(controlPointIndex).mAlpha;
				}
				break;
			
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexColor->GetIndexArray().GetAt(controlPointIndex);
					color->x = (float)vertexColor->GetDirectArray().GetAt(id).mRed;
					color->y = (float)vertexColor->GetDirectArray().GetAt(id).mGreen;
					color->z = (float)vertexColor->GetDirectArray().GetAt(id).mBlue;
					color->w = (float)vertexColor->GetDirectArray().GetAt(id).mAlpha;
				}
				break;
				
			default:
				break;
			}
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		{
			switch(vertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					color->x = (float)vertexColor->GetDirectArray().GetAt(vertexCounter).mRed;
					color->y = (float)vertexColor->GetDirectArray().GetAt(vertexCounter).mGreen;
					color->z = (float)vertexColor->GetDirectArray().GetAt(vertexCounter).mBlue;
					color->w = (float)vertexColor->GetDirectArray().GetAt(vertexCounter).mAlpha;
				}
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexColor->GetIndexArray().GetAt(vertexCounter);
					color->x = (float)vertexColor->GetDirectArray().GetAt(id).mRed;
					color->y = (float)vertexColor->GetDirectArray().GetAt(id).mGreen;
					color->z = (float)vertexColor->GetDirectArray().GetAt(id).mBlue;
					color->w = (float)vertexColor->GetDirectArray().GetAt(id).mAlpha;
				}
				break;
			default:
				break;
			}
		}
		break;
	}
}

void fbxLoader::readUV(FbxMesh* mesh, int controlPointIndex, int uvLayer, D3DXVECTOR2* uv)
{
	if (uvLayer >= 2 || mesh->GetElementUVCount() <= uvLayer)
	{
		return;
	}

	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(uvLayer);
	
	switch(vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		{
			switch(vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					uv->x = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[0];
					uv->y = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexUV->GetIndexArray().GetAt(controlPointIndex);
					uv->x = (float)vertexUV->GetDirectArray().GetAt(id).mData[0];
					uv->y = (float)vertexUV->GetDirectArray().GetAt(id).mData[1];
				}
				break;
			}
		}
		break;

	case FbxGeometryElement::ePolygonGroup:
		{
			switch(vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					uv->x = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[0];
					uv->y = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = vertexUV->GetIndexArray().GetAt(controlPointIndex);
					uv->x = (float)vertexUV->GetDirectArray().GetAt(id).mData[0];
					uv->y = (float)vertexUV->GetDirectArray().GetAt(id).mData[1];
				}
				break;
			}
		}
		break;

		default:
			int id = vertexUV->GetIndexArray().GetAt(controlPointIndex);
			uv->x = (float)vertexUV->GetDirectArray().GetAt(id).mData[0];
			uv->y = (float)vertexUV->GetDirectArray().GetAt(id).mData[1];
			//vertexUV->GetIndexArray().GetAt(controlPointIndex).mData[0];
	}
}

void fbxLoader::readNormal(FbxMesh* mesh, int controlPointIndex, D3DXVECTOR3* normal)
{
	if (mesh->GetElementNormalCount()<1)
	{
		return;
	}

	FbxGeometryElementNormal* normalEl = mesh->GetElementNormal(0);

	switch(normalEl->GetMappingMode())
	{
	case FbxGeometryElement::eDirect:
		{
			switch(normalEl->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					normal->x = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[0];
					normal->y = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[1];
					normal->z = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[2];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = normalEl->GetIndexArray().GetAt(controlPointIndex);
					normal->x = (float)normalEl->GetDirectArray().GetAt(id).mData[0];
					normal->y = (float)normalEl->GetDirectArray().GetAt(id).mData[1];
					normal->z = (float)normalEl->GetDirectArray().GetAt(id).mData[2];
				}
				break;
			}
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		{
			switch(normalEl->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					normal->x = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[0];
					normal->y = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[1];
					normal->z = (float)normalEl->GetDirectArray().GetAt(controlPointIndex).mData[2];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = normalEl->GetIndexArray().GetAt(controlPointIndex);
					normal->x = (float)normalEl->GetDirectArray().GetAt(id).mData[0];
					normal->y = (float)normalEl->GetDirectArray().GetAt(id).mData[1];
					normal->z = (float)normalEl->GetDirectArray().GetAt(id).mData[2];
				}
				break;
			}
		}
		break;
	default:
		int id = normalEl->GetIndexArray().GetAt(controlPointIndex);
		normal->x = (float)normalEl->GetDirectArray().GetAt(id).mData[0];
		normal->y = (float)normalEl->GetDirectArray().GetAt(id).mData[1];
		normal->z = (float)normalEl->GetDirectArray().GetAt(id).mData[2];
	}
}

void fbxLoader::readTangent(FbxMesh* mesh, int controlPointIndex, int vertexCounter, D3DXVECTOR3* tangent)
{
	if(mesh->GetElementTangentCount() < 1)
	{
		return;
	}

	FbxGeometryElementTangent* tangentEl = mesh->GetElementTangent(0);

	switch(tangentEl->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		{
			switch(tangentEl->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					tangent->x = (float)tangentEl->GetDirectArray().GetAt(controlPointIndex).mData[0];
					tangent->y = (float)tangentEl->GetDirectArray().GetAt(controlPointIndex).mData[1];
					tangent->z = (float)tangentEl->GetDirectArray().GetAt(controlPointIndex).mData[2];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = tangentEl->GetIndexArray().GetAt(controlPointIndex);
					tangent->x = (float)tangentEl->GetDirectArray().GetAt(id).mData[0];
					tangent->y = (float)tangentEl->GetDirectArray().GetAt(id).mData[1];
					tangent->z = (float)tangentEl->GetDirectArray().GetAt(id).mData[2];
				}
				break;
			}
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		{
			switch(tangentEl->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					tangent->x = (float)tangentEl->GetDirectArray().GetAt(vertexCounter).mData[0];
					tangent->y = (float)tangentEl->GetDirectArray().GetAt(vertexCounter).mData[1];
					tangent->z = (float)tangentEl->GetDirectArray().GetAt(vertexCounter).mData[2];
				}
				break;

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = tangentEl->GetIndexArray().GetAt(vertexCounter);
					tangent->x = (float)tangentEl->GetDirectArray().GetAt(id).mData[0];
					tangent->y = (float)tangentEl->GetDirectArray().GetAt(id).mData[1];
					tangent->z = (float)tangentEl->GetDirectArray().GetAt(id).mData[2];
				}
				break;
			}
		}
		break;
	}
}

void fbxLoader::buildSkeletonCycle(FbxNode* boneNode, int parentBoneIndex)
{
	BoneData* bone1 = new BoneData();
	bone1->SetName(boneNode->GetName());
	bone1->SetID(skeleton->GetBonesCount());
	bone1->SetParent(parentBoneIndex);

	skeleton->SetBones(bone1);

	int childCount = boneNode->GetChildCount();

	for( int childIndex = 0; childIndex < childCount; ++childIndex )
	{
		FbxNode* childNode = boneNode->GetChild( childIndex );
		std::string s2("Nub");
		std::string s3 ("Footsteps");
		std::string s1(childNode->GetName());
		int found = s1.find(s2);
		int found2 = s1.find(s3);
		if (found == std::string::npos && found2 == std::string::npos)
		{
			buildSkeletonCycle(childNode, bone1->GetID());
		}
	}
}

void fbxLoader::buildSkeleton(FbxNode *node)
{
	std::string s2("Nub");
	std::string s3("Footsteps");
	std::string s1(node->GetName());
	int found = s1.find(s2);
	int found2 = s1.find(s3);

	if( node->GetNodeAttribute())
	{
		switch(node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eSkeleton:
			if (found == std::string::npos && found2 == std::string::npos)
			{
				if(this->findSkeletonRootBone(node) && rootbone == false) 
				{
					FbxSkeleton *skeletalMesh = node->GetSkeleton();
					if( !skeletalMesh ) return;
					FbxNode* boneNode = skeletalMesh->GetNode();
					buildSkeletonCycle(boneNode, -1);
					rootbone1 = true;
				}
			}
			break;

		}
	}

	for (int i = 0; i<node->GetChildCount(); i++)
	{
		FbxNode *child = node->GetChild(i);
		//buildSkeleton(child);
	}

	
}

FbxNode* fbxLoader::findSkeletonRootBone(FbxNode* node)
{
	FbxNode* parent = node->GetParent();

	if (IsMeshSkeleton(node) && parent && !IsMeshSkeleton(parent))
	{
		return node;
	}
	
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		FbxNode * result = findSkeletonRootBone(node->GetChild(i));
		if (result != nullptr)
		{
			return result;
		}
	}
}

bool fbxLoader::IsMeshSkeleton(FbxNode* node)
{
	FbxNodeAttribute * attrib = node->GetNodeAttribute();
	if( !attrib ) return false;

	bool result = attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton && !((FbxSkeleton*) attrib)->GetSkeletonType() == FbxSkeleton::eLimbNode;

	return result;
}

void fbxLoader::setBindPose(FbxNode* node)
{
	const char * nodename = node->GetName();
	scaleBones = FbxVector4(0,0,0,0);

	for (int i = 0; i<scene->GetPoseCount(); i++)
	{
		FbxPose* pose = scene->GetPose(i);

		if (pose)
		{
			int lNodeIndex = pose->Find(node);
			if(lNodeIndex > -1)
			{
				if( pose->IsBindPose() || !pose->IsLocalMatrix(lNodeIndex))
				{
					FbxMatrix poseMatrix = pose->GetMatrix(lNodeIndex);
					FbxQuaternion rotationQ = FbxQuaternion (0,0,0,0);
					double sign = 0;
					FbxVector4 translation = FbxVector4(0,0,0,0);
					FbxVector4 shearing = FbxVector4(0,0,0,0);
					FbxVector4 scale = FbxVector4(0,0,0,0);
					
					std::string namethis = node->GetName();
					int idBone = skeleton->GetBoneByName(node->GetName());
					poseMatrix.GetElements(translation, rotationQ, shearing, scale, sign);
					if(scale.mData[0] > scaleBones.mData[0])
						scaleBones.mData[0] = scale.mData[0];
					if(scale.mData[1] > scaleBones.mData[1])
						scaleBones.mData[1] = scale.mData[1];
					if(scale.mData[2] > scaleBones.mData[2])
						scaleBones.mData[2] = scale.mData[2];

					D3DXQUATERNION rotationQuat = D3DXQUATERNION(rotationQ.mData[0], rotationQ.mData[1], rotationQ.mData[2], rotationQ.mData[3]);
					skeleton->GetBone(idBone)->SetLocalOrient(rotationQuat);

					skeleton->GetBone(idBone)->SetLocalPos(D3DXVECTOR3(translation.mData[0], translation.mData[1], translation.mData[2])); ///right now i scale it down 20 times, but previously it was 10.... 

					D3DXMATRIX copymatrix = D3DXMATRIX( poseMatrix.mData[0].mData[0], poseMatrix.mData[0].mData[1], poseMatrix.mData[0].mData[2], poseMatrix.mData[0].mData[3],
					poseMatrix.mData[1].mData[0], poseMatrix.mData[1].mData[1], poseMatrix.mData[1].mData[2], poseMatrix.mData[1].mData[3],
					poseMatrix.mData[2].mData[0], poseMatrix.mData[2].mData[1], poseMatrix.mData[2].mData[2], poseMatrix.mData[2].mData[3],
					poseMatrix.mData[3].mData[0], poseMatrix.mData[3].mData[1], poseMatrix.mData[3].mData[2], poseMatrix.mData[3].mData[3]);

					skeleton->GetBone(idBone)->SetTransformation(copymatrix);
				}
			}
		}
	}
}

void fbxLoader::setBindPoseCluster(FbxNode *node)
{
	if( node->GetNodeAttribute())
	{
		switch(node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh: 
			FbxMesh *mesh = node->GetMesh();

			for (int j = 0; j<mesh->GetDeformerCount(); j++)
			{
				FbxSkin *skin = (FbxSkin*) mesh->GetDeformer(j,FbxDeformer::eSkin);
				int clusters = skin->GetClusterCount();
				for(int k = 0; k<clusters; k++)
				{
					FbxCluster* cluster = skin->GetCluster(k);
					FbxNode* boneLink = cluster->GetLink();

					if(boneLink)
					{
						std::string nameLink = boneLink->GetName();
						FbxAMatrix translationM;
						FbxAMatrix invert;
						cluster->GetTransformLinkMatrix(translationM);
						cluster->GetTransformMatrix(invert);

						translationM = translationM * invert.Inverse();

						D3DXMATRIX mat = D3DXMATRIX((float)translationM.mData[0].mData[0], (float)translationM.mData[0].mData[1], -(float)translationM.mData[0].mData[2], (float)translationM.mData[3].mData[0],
													(float)translationM.mData[1].mData[0],(float) translationM.mData[1].mData[1], -(float)translationM.mData[1].mData[2], (float)translationM.mData[3].mData[1],
													(float)translationM.mData[2].mData[0], (float)translationM.mData[2].mData[1], -(float)translationM.mData[2].mData[2],(float)translationM.mData[3].mData[2],
													0,0,0,1);

						skeleton->GetBone(skeleton->GetBoneByName(nameLink))->SetTransformation(mat);
					}
				}
			}
			break;
		}
	}
	for (int i = 0; i<node->GetChildCount(); i++)
	{
		FbxNode* child = node->GetChild(i);
		setBindPoseCluster(child);
	}
}

void fbxLoader::buildAnimation(FbxSkeleton* skeletalMesh)
{
	if( !skeletalMesh ) return;
	FbxNode* boneNode = skeletalMesh->GetNode();
	readAnimationTakes(boneNode);
}

void fbxLoader::readAnimationWeigths(FbxMesh* mesh)
{
	if(!animationStructure)
	{
		animationStructure = new AnimationData();
	}

	int numSkins = mesh->GetDeformerCount(FbxDeformer::eSkin);

	weightsNumber = 0;

	//wiem że to nie ma sensu, ale już lepiej dłużej chwilę wczytywać dane niż się kopać z koniem
	/*for(int i = 0; i < numSkins; i++)
	{
		FbxSkin* skin = (FbxSkin*) mesh->GetDeformer(i, FbxDeformer::eSkin);

		int clusterCount = skin->GetClusterCount();
		if( clusterCount == 0 ) continue;
		for (int j = 0; j < clusterCount; j++)
		{
			FbxCluster* cluster = skin->GetCluster(j);///trzeba jakoś zrobić wszystkie linki... edit, have no idea what i meant xD
			FbxNode* bone = cluster->GetLink();

			if( !bone )
			{
				continue;
			}

			int numInfluencedVertices = cluster->GetControlPointIndicesCount();

			for(int iControlPoint = 0; iControlPoint < numInfluencedVertices; iControlPoint++)
			{
				weightsNumber++;
			}
		}
	}*/

	//weights = new weightsStructure*[weightsNumber];

	for (int l = 0; l<this->vertexMaxCount; l++)
	{
		for(int p = 0; p<4; p++)
		{
			this->vertexArray[l].weights[p].boneID = 0;
			this->vertexArray[l].weights[p].weight = 0.0f;
		}
	}

	int next = 0;
	int numindicestest = 0;

	for(int i = 0; i < numSkins; i++)
	{
		FbxSkin* skin = (FbxSkin*) mesh->GetDeformer(i, FbxDeformer::eSkin);

		int clusterCount = skin->GetClusterCount();
		if( clusterCount == 0 ) continue;

		for (int j = 0; j < clusterCount; j++)
		{
			FbxCluster* cluster = skin->GetCluster(j);///trzeba jakoś zrobić wszystkie linki... edit, have no idea what i meant xD
			FbxNode* bone = cluster->GetLink();

			if( !bone )
			{
				continue;
			}

			int numInfluencedVertices = cluster->GetControlPointIndicesCount();

			int* pIndexArray = cluster->GetControlPointIndices();
			numindicestest += cluster->GetControlPointIndicesCount();
			
			double *pWeightArray = cluster->GetControlPointWeights();


			for(int iControlPoint = 0; iControlPoint < numInfluencedVertices; iControlPoint++)
			{
				/*int iControlPointIndex = pIndexArray[iControlPoint];
				////////////muszę poprawić indeksy wierzchołków. teraz odwoluje sie do vertow afaik
				int boneIndex = skeleton->GetBoneByName(bone->GetName());
				int boneI = skeleton->GetBoneIndex(boneIndex);
				//boneIndices[iControlPointIndex] = iCluster;
				double weightAdd = pWeightArray[iControlPoint];

				weightsStructure* added = new weightsStructure(boneI, iControlPointIndex, weightAdd);

				this->weights[next] = added;
				next++;*/

				int iControlPointIndex = pIndexArray[iControlPoint];

				int boneIndex = skeleton->GetBoneByName(bone->GetName());
				int k = 0;

				if(vertexArray[iControlPointIndex].weights[0].weight == 0.0f)
				{
					vertexArray[iControlPointIndex].weights[0].boneID = skeleton->GetBoneIndex(boneIndex);
					vertexArray[iControlPointIndex].weights[0].weight = pWeightArray[iControlPoint];
					vertexArray[iControlPointIndex].weightsNum = 1;
				}
				else
				{
					while (k < 3 && vertexArray[iControlPointIndex].weights[k].weight != 0.0f)
					{
						k++;
					}
					vertexArray[iControlPointIndex].weights[k].boneID = skeleton->GetBoneIndex(boneIndex);
					vertexArray[iControlPointIndex].weights[k].weight = pWeightArray[iControlPoint];
					vertexArray[iControlPointIndex].weightsNum++;
				}
			}
		}
	}
}

void fbxLoader::ComputeClusterDeformation(FbxMesh* pMesh, FbxAMatrix &pGlobalPosition, FbxCluster* pCluster, FbxAMatrix &transformationM, const FbxTime& time, FbxPose* pose)
{
    FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	FbxAMatrix lReferenceGlobalInitPosition;
	FbxAMatrix lReferenceGlobalCurrentPosition;
	FbxAMatrix lAssociateGlobalInitPosition;
	FbxAMatrix lAssociateGlobalCurrentPosition;
	FbxAMatrix lClusterGlobalInitPosition;
	FbxAMatrix lClusterGlobalCurrentPosition;

	FbxAMatrix lReferenceGeometry;
	FbxAMatrix lAssociateGeometry;
	FbxAMatrix lClusterGeometry;

	FbxAMatrix lClusterRelativeInitPosition;
	FbxAMatrix lClusterRelativeCurrentPositionInverse;
	
	if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
	{
		pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
		// Geometric transform of the model
		lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), time, pose);

		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		//lReferenceGlobalCurrentPosition = pGlobalPosition;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = GetGeometry(pCluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), time, pose);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		transformationM = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = pGlobalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), time, pose);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		transformationM = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

void fbxLoader::ComputeDualQuaternionDeformation(FbxMesh* pMesh, FbxAMatrix &pGlobalPosition, const FbxTime& frame, int frameID)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[this->getIndexMaxCount()];
	memset(lDQClusterDeformation, 0, this->getIndexMaxCount() * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[this->getIndexMaxCount()];
	memset(lClusterWeight, 0, this->getIndexMaxCount() * sizeof(double));

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pMesh, pGlobalPosition, lCluster, lVertexTransformMatrix, frame, scene->GetPose(0)); // poprawnie to odpalic i wrzucic poprawne macierze (miejmy nadzieje ze poprawne)

			D3DXMATRIX copymatrix = D3DXMATRIX((float)lVertexTransformMatrix.mData[0].mData[0], (float)lVertexTransformMatrix.mData[1].mData[0], (float)lVertexTransformMatrix.mData[2].mData[0], lVertexTransformMatrix.mData[3].mData[0],
								(float)lVertexTransformMatrix.mData[0].mData[1],(float) lVertexTransformMatrix.mData[1].mData[1], (float)lVertexTransformMatrix.mData[2].mData[1], lVertexTransformMatrix.mData[3].mData[1],
								(float)lVertexTransformMatrix.mData[0].mData[2], (float)lVertexTransformMatrix.mData[1].mData[2], (float)lVertexTransformMatrix.mData[2].mData[2], lVertexTransformMatrix.mData[3].mData[2],
								lVertexTransformMatrix.mData[0].mData[3],lVertexTransformMatrix.mData[1].mData[3],lVertexTransformMatrix.mData[2].mData[3],1);

			std::string nm = lCluster->GetLink()->GetName();
			animationStructure->GetSkeleton(frameID)->GetBone(animationStructure->GetSkeleton(frameID)->GetBoneByName(lCluster->GetLink()->GetName()))->SetTransformation(copymatrix);

			int testBone = animationStructure->GetSkeleton(frameID)->GetBoneByName(lCluster->GetLink()->GetName());

			std::string s1 ("Spine3");
			int found = nm.find(s1);		
			if (found != std::string::npos)
			{
				copymatrix;
				int checkcop = 0;
			}

			/*FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{ 
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if(lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if( lSign >= 0.0 )
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	/*for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeightSum = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeightSum != 0.0) 
		{
			lDQClusterDeformation[i].Normalize();
			lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeightSum;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeightSum);
				lDstVertex += lSrcVertex;
			}
		} 
	}*/

		}
	}
	delete [] lDQClusterDeformation;
	delete [] lClusterWeight;
}

void fbxLoader::readAnimationTakes(FbxNode* node)
{
	//testpositioning+=0.2f;
	FbxPose* pose = scene->GetPose(0);
	FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(FBX_TYPE(FbxAnimStack)));
	int numAnimLayers = pAnimStack->GetMemberCount(FBX_TYPE(FbxAnimLayer));


	for (int i = 0; i < numAnimLayers; i++)
	{
		FbxAnimLayer* pAnimLayer = pAnimStack->GetMember(FBX_TYPE(FbxAnimLayer), i);

		FbxAnimCurve* animCv = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);


		if (animCv)
		{
			int p = animCv->KeyGetCount();

			for (int j = 0; j<p; j++)
			{
				D3DXMATRIX clearmatrix = D3DXMATRIX(0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0);
				animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->SetTransformation(clearmatrix);
				FbxMatrix poseMatrix = pose->GetMatrix(pose->Find(node));

				FbxAnimCurveKey* animKey = &animCv->KeyGet(j);

				const FbxVector4 keyTranslation = scene->GetEvaluator()->GetNodeLocalTranslation(node,animKey->GetTime());
				FbxVector4 keyGlobalTranslation = scene->GetEvaluator()->GetNodeGlobalTransform(node, animKey->GetTime());
				FbxVector4 linkScale = scene->GetEvaluator()->GetNodeLocalScaling(node, animKey->GetTime());

				FbxVector4 keyRotation = scene->GetEvaluator()->GetNodeLocalRotation(node, animKey->GetTime());
				FbxVector4 keyGlobalRotation = scene->GetEvaluator()->GetNodeLocalRotation(node, animKey->GetTime());

				const FbxQuaternion rot = FbxQuaternion(keyRotation.mData[0], keyRotation.mData[1], -keyRotation.mData[2], keyRotation.mData[3]);
				const FbxVector4 scale = scene->GetEvaluator()->GetNodeLocalScaling(node, animKey->GetTime());

				FbxMatrix transl = node->EvaluateLocalTransform(animKey->GetTime());
				FbxMatrix poseM = scene->GetPose(0)->GetMatrix(scene->GetPose(0)->Find(node));

				FbxAMatrix localAMatrix = FbxAMatrix(scene->GetEvaluator()->GetNodeLocalTranslation(node), scene->GetEvaluator()->GetNodeLocalRotation(node), scene->GetEvaluator()->GetNodeLocalScaling(node));
				FbxAMatrix globalAMatrix = FbxAMatrix(scene->GetEvaluator()->GetNodeGlobalTransform(node));

				poseM = FbxMatrix((float)poseM.mData[0].mData[0], (float)poseM.mData[1].mData[0], (float)poseM.mData[2].mData[0], 0,
								(float)poseM.mData[0].mData[1],(float) poseM.mData[1].mData[1], (float)poseM.mData[2].mData[1], 0,
								(float)poseM.mData[0].mData[2], (float)poseM.mData[1].mData[2], (float)poseM.mData[2].mData[2], 0,
								0,0,0,1);
				D3DXMATRIX copymatrix;
			
				//transl = transl*poseM;

				if(animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->GetID() == 0)
				{
					transl = scene->GetEvaluator()->GetNodeGlobalTransform(node);
					transl = transl.Transpose();
					continue;
				}
				else
				{
					FbxMatrix rootMatrix = FbxMatrix(animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._11, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._12, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._13, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._14,
													animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._21, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._22, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._23, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._24,
													animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._31, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._32, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._33, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._34,
													animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._41, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._42, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._43, animationStructure->GetSkeleton(j)->GetBone(0)->GetTransformation()._44);
					//FbxMatrix baseTransform = node->GetTransform();
					FbxMatrix globalCurrent = node->EvaluateGlobalTransform(animKey->GetTime());
					FbxMatrix baseTransform = scene->GetEvaluator()->GetNodeGlobalTransform(node);
					
					FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
					FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
					FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

					FbxMatrix referenceGeometry = FbxMatrix(lT, lR, lS);

					baseTransform *= referenceGeometry;

					FbxMatrix globalPosition = GetGlobalPosition(node, animKey->GetTime(), scene->GetPose(0));

					baseTransform *= globalPosition;

					transl = FbxMatrix(globalCurrent);
					//transl *= invertroot;
				}
				/////////////////skopiować to co jest w ComputeClusterDeformation

				if(animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->GetID() > 0)
				{
					copymatrix = D3DXMATRIX( (float)transl.mData[0].mData[0], (float)transl.mData[1].mData[0], (float)transl.mData[2].mData[0], (float)transl.mData[3].mData[0],//keyTranslation.mData[0],
											(float)transl.mData[0].mData[1],(float) transl.mData[1].mData[1], (float)transl.mData[2].mData[1], (float)transl.mData[3].mData[1],// keyTranslation.mData[1],
											(float)transl.mData[0].mData[2], (float)transl.mData[1].mData[2], (float)transl.mData[2].mData[2], (float)transl.mData[3].mData[2],//keyTranslation.mData[2],
											0,0,0,1);
				}
				else 
				{
					copymatrix = D3DXMATRIX( (float)transl.mData[0].mData[0], (float)transl.mData[1].mData[0], (float)transl.mData[2].mData[0], (float)transl.mData[3].mData[0],//keyTranslation.mData[0],
											(float)transl.mData[0].mData[1], (float) transl.mData[1].mData[1], (float)transl.mData[2].mData[1], (float)transl.mData[3].mData[1],// keyTranslation.mData[1],
											(float)transl.mData[0].mData[2], (float)transl.mData[1].mData[2], (float)transl.mData[2].mData[2], (float)transl.mData[3].mData[2],//keyTranslation.mData[2],
											0,0,0,1);//(float)transl.mData[0].mData[3], (float)transl.mData[1].mData[3], (float)transl.mData[2].mData[3], (float)transl.mData[3].mData[3]);
				}

				copymatrix = copymatrix;
									
				
				//copymatrix = D3DXMATRIX(
					//1,   0,   0,    keyTranslation.mData[0],
					//0, 1,     0,      keyTranslation.mData[1],
					//0,0, 1,          keyTranslation.mData[2],
					//1,1,1,1);

				animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->SetLocalOrient(D3DXQUATERNION(keyRotation.mData[0], keyRotation.mData[1], keyRotation.mData[2], keyRotation.mData[3]));
				animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->SetLocalPos(D3DXVECTOR3(keyGlobalTranslation.mData[0], keyGlobalTranslation.mData[1], keyGlobalTranslation.mData[2]));
				animationStructure->GetSkeleton(j)->GetBone(animationStructure->GetSkeleton(j)->GetBoneByName(node->GetName()))->SetTransformation(copymatrix);
			}
		}
	}

	int childCount = node->GetChildCount();

	for( int childIndex = 0; childIndex < childCount; ++childIndex )
	{
		FbxNode* childNode = node->GetChild( childIndex );
		std::string s2("Nub");
		std::string s1(childNode->GetName());
		int found = s1.find(s2);
		if (found == std::string::npos)
		{
			readAnimationTakes(childNode);
		}
	}

	/*for (int i = 0; i < scene->GetSrcObjectCount(FBX_TYPE(FbxAnimStack)); i++)
	{
		 FbxAnimStack* lAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(FBX_TYPE(FbxAnimStack), i));

		 int nbAnimLayers = lAnimStack->GetMemberCount(FBX_TYPE(FbxAnimLayer));
		 for (int l = 0; l < nbAnimLayers; l++)
		 {
			 FbxAnimLayer* lAnimLayer = lAnimStack->GetMember(FBX_TYPE(FbxAnimLayer), l);
			 FbxAnimCurve* lAnimCurve = node->LclTranslation(GetCurve<FbxAnimCurve>(lAnimLayer, FCURVENODE_T_X));
			 if(lAnimCurve)
			 {
				 int lKeyCount = lAnimCurve->KeyGetCount();
				 for(int k = 0; k<lKeyCount; ++k)
				 {
					 FbxAnimCurveKey lKey = lAnimCurve->KeyGet(k);
					 float lKeyValue = lKey.GetValue();
				 }
			 }
		  }
	 }*/
}

void fbxLoader::readAnimationTakeData(FbxNode* node)
{
	FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(FBX_TYPE(FbxAnimStack)));
	FbxAnimLayer* pAnimLayer = pAnimStack->GetMember(FBX_TYPE(FbxAnimLayer));

	FbxAnimCurve* animCv = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

	FbxTimeSpan length = FbxTimeSpan();

	int p = animCv->KeyGetCount();

	const char* nameAnim = animCv->GetName();
	const size_t len = strlen(nameAnim);
	char * new_name = new char[len + 1];
	strncpy(new_name, nameAnim, len);

	animCv->GetTimeInterval(length);

	FbxTime duration = length.GetDuration();
	FbxTime::EMode mode = duration.GetGlobalTimeMode();
	double frameRate = duration.GetFrameRate(mode);

	double startt = length.GetStart().GetMilliSeconds();
	double endt = length.GetStop().GetMilliSeconds();

	int frames = animCv->KeyGetCount();

	animationStructure = new AnimationData(new_name, startt, endt, (int)frameRate, frames);

	for (int i = 0; i< frames; i++)
	{
		SkeletalData *sk = new SkeletalData();
		for(int j = 0; j<skeleton->GetBonesCount(); j++)
		{
			BoneData *bonecopy = new BoneData();
			bonecopy->SetID(skeleton->GetBone(j)->GetID());
			bonecopy->SetName(skeleton->GetBone(j)->GetName());
			bonecopy->SetParent(skeleton->GetBone(j)->GetParent());
			sk->SetBones(bonecopy);
		}
		animationStructure->SetSkeleton(sk, i);
	}
}

D3DXVECTOR4 fbxLoader::getRotationVector(FbxMatrix colMatrix)
{
	double sinPitch, cosPitch, sinRoll, cosRoll, sinYaw, cosYaw;
 
	sinPitch = -colMatrix[2][0];
	cosPitch = sqrt(1 - sinPitch*sinPitch);
 
	if ( abs(cosPitch) > DBL_MIN ) 
	{
	    sinRoll = colMatrix[2][1] / cosPitch;
	    cosRoll = colMatrix[2][2] / cosPitch;
	    sinYaw = colMatrix[1][0] / cosPitch;
	    cosYaw = colMatrix[0][0] / cosPitch;
    } 
    else 
    {
	    sinRoll = -colMatrix[1][2];
	    cosRoll = colMatrix[1][1];
	    sinYaw = 0;
	    cosYaw = 1;
    }

	return D3DXVECTOR4(atan2(sinRoll, cosRoll) * 180 / D3DX_PI, atan2(sinPitch, cosPitch) * 180 / D3DX_PI, atan2(sinYaw, cosYaw) * 180 / D3DX_PI, 1);
}

void fbxLoader::convertScale()
{
	for(int i = 0; i< animationStructure->GetFramesNumber(); i++)
	{
		for (int j = 0; j< skeleton->GetBonesCount(); j++)
		{
			D3DXMATRIX converted = D3DXMATRIX(animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._11, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._12, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._13, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._14/this->getSc().x,
				animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._21, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._22, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._23, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._24/this->getSc().y,
				animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._31, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._32, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._33, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._34/-this->getSc().z/2,
				animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._41, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._42, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._43, animationStructure->GetSkeleton(i)->GetBone(j)->GetTransformation()._44);

			animationStructure->GetSkeleton(i)->GetBone(j)->SetTransformation(converted);
		}
	}
}

FbxAMatrix fbxLoader::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition;
    bool        lPositionFound = false;

    if (pPose)
    {
        int lNodeIndex = pPose->Find(pNode);

        if (lNodeIndex > -1)
        {
            // The bind pose is always a global matrix.
            // If we have a rest pose, we need to check if it is
            // stored in global or local space.
            if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
            {
                lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
            }
            else
            {
                // We have a local matrix, we need to convert it to
                // a global space matrix.
                FbxAMatrix lParentGlobalPosition;

                if (pParentGlobalPosition)
                {
                    lParentGlobalPosition = *pParentGlobalPosition;
                }
                else
                {
                    if (pNode->GetParent())
                    {
                        lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
                    }
                }

                FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
                lGlobalPosition = lParentGlobalPosition * lLocalPosition;
            }

            lPositionFound = true;
        }
    }

    if (!lPositionFound)
    {
        // There is no pose entry for that node, get the current global position instead.

        // Ideally this would use parent global position and local position to compute the global position.
        // Unfortunately the equation 
        //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
        // does not hold when inheritance type is other than "Parent" (RSrs).
        // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
        lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
    }

    return lGlobalPosition;
}

FbxAMatrix fbxLoader::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

FbxAMatrix fbxLoader::GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}




//draw node recursive ( na koniec odpala dla dzieci node'a)
//draw node sprawdza jaki to node i przekazuje do draw
//
//skeleton jest niewazne
//
//draw mesh najpierw normalnie z mesha->getcontrolpoints
//
// normalnie po kazdym clustrze
// 
// trzeba skopiowac compute skin deformation z wszystkimi compute dual quaternion itp.!
//!!!!!!!!!!!!!!!!!!!!!