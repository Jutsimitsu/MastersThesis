#include "fbxLoader2.h"


fbxLoader2::fbxLoader2(void)
{
}


fbxLoader2::~fbxLoader2(void)
{
}


fbxLoader2::fbxLoader2(char* filename)
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
	
	fbxSkeleton = 0;
	skeleton = 0;
	animationStructure = 0;

	loadFbx(filename);
}

void fbxLoader2::Shutdown()
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

void fbxLoader2::loadFbx(char* filename)
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
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);

	lImporter->Import(scene);
	lImporter->Destroy();

	lRootNode = scene->GetRootNode();

	//int numberOfChildren = lRootNode->GetChildCount();
	skeleton = new SkeletalData();
	animationStructure = new AnimationData();
	buildSkeleton(findSkeletonRootBone(lRootNode));
	readAnimationTakeData(findSkeletonRootBone(lRootNode)->GetChild(0));
	readVertexData(lRootNode);
	
	indices = new indexesStructure[indicesMaxCount];
	vertexArray = new vertexWeights[vertexMaxCount];

	processNode(lRootNode);

	//processMesh(lRootNode);


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

void fbxLoader2::processNode(FbxNode* node)
{
	//FbxNodeAttribute::EType attributeType;
	if(node->GetNodeAttribute())
	{
		switch(node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			processMesh(node);
			break;

		case FbxNodeAttribute::eSkeleton:
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

void fbxLoader2::processMesh(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();

	this->readAnimationWeigths(mesh);

	if(mesh!=NULL && mesh->IsTriangleMesh())
	{
		for (int i = 0; i<mesh->GetControlPointsCount(); i++)
		{
			readVertex(mesh, i, &vertex[i]);
			vertexArray[i].position = D3DXVECTOR3(vertex[i]);
		}

		int a = mesh->GetPolygonVertexCount();

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

	//vertexLists.push_back(vertexArray);
	indiceLists.push_back(indices);

	FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(FBX_TYPE(FbxAnimStack)));
	int numAnimLayers = pAnimStack->GetMemberCount(FBX_TYPE(FbxAnimLayer));

	this->setBindPoseCluster(node);

	for (int i = 0; i < numAnimLayers; i++)
	{
		FbxAnimLayer* pAnimLayer = pAnimStack->GetMember(FBX_TYPE(FbxAnimLayer), i);
		FbxAnimCurve* animCv = this->findSkeletonRootBone(scene->GetRootNode())->GetChild(0)->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (animCv)
		{
			FbxTimeSpan animationLength;
			int p = animCv->KeyGetCount();
			animCv->GetTimeInterval(animationLength);

			for(int j = 0; j<animationStructure->GetFramesNumber();j++)
			{
				FbxTime timeKey = animCv->KeyGet(j).mTime;
				//FbxTime interval = (duration/p)*j + animationLength.GetStart();

				//int intervalVal = (duration.GetSecondCount()/p)*j + animationLength.GetStart().GetSecondCount();
				const FbxTime pTime = animCv->KeyGet(j).mTime;


				FbxAMatrix pGlobalPos = GetGlobalPosition(node, pTime, scene->GetPose(j));

				ComputeSkinDeformation(pGlobalPos, mesh, timeKey, scene->GetPose(j), j);
			}
		}
	}

	for(int i = 0; i<node->GetChildCount(); i++)
	{
		processMesh(node->GetChild(i));
	}
}

void fbxLoader2::readVertexData(FbxNode* node)
{
	if(node->GetNodeAttribute())
	{
		switch(node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			vertexMaxCount += node->GetMesh()->GetControlPointsCount();
			indicesMaxCount += node->GetMesh()->GetPolygonVertexCount();
			printf("");
			break;
		}
	}

	for(int i = 0; i<node->GetChildCount(); i++)
	{
		readVertexData(node->GetChild(i));
	}
}

void fbxLoader2::readVertex(FbxMesh* mesh, int controlPointIndex, D3DXVECTOR3* vertex)
{
	FbxVector4* ctrlPoint = mesh->GetControlPoints();

	vertex->x = (float)ctrlPoint[controlPointIndex].mData[0];
	vertex->y = (float)ctrlPoint[controlPointIndex].mData[1];
	vertex->z = (float)ctrlPoint[controlPointIndex].mData[2];
}

void fbxLoader2::readColor(FbxMesh* mesh, int controlPointIndex, int vertexCounter, D3DXVECTOR4* color)
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

void fbxLoader2::readUV(FbxMesh* mesh, int controlPointIndex, int uvLayer, D3DXVECTOR2* uv)
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

void fbxLoader2::readNormal(FbxMesh* mesh, int controlPointIndex, D3DXVECTOR3* normal)
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

void fbxLoader2::readTangent(FbxMesh* mesh, int controlPointIndex, int vertexCounter, D3DXVECTOR3* tangent)
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

void fbxLoader2::readAnimationTakeData(FbxNode* node)
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

void fbxLoader2::setBindPoseCluster(FbxNode *node)
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

						D3DXMATRIX mat = D3DXMATRIX((float)translationM.mData[0].mData[0], (float)translationM.mData[0].mData[1], (float)translationM.mData[0].mData[2], (float)translationM.mData[3].mData[0],
													(float)translationM.mData[1].mData[0], (float)translationM.mData[1].mData[1], (float)translationM.mData[1].mData[2], (float)translationM.mData[3].mData[1],
													(float)translationM.mData[2].mData[0], (float)translationM.mData[2].mData[1], (float)translationM.mData[2].mData[2], (float)translationM.mData[3].mData[2],
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

void fbxLoader2::readAnimationWeigths(FbxMesh* mesh)
{
	if(!animationStructure)
	{
		animationStructure = new AnimationData();
	}

	int numSkins = mesh->GetDeformerCount(FbxDeformer::eSkin);

	weightsNumber = 0;

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

bool fbxLoader2::IsMeshSkeleton(FbxNode* node)
{
	FbxNodeAttribute * attrib = node->GetNodeAttribute();
	if( !attrib ) return false;

	bool result = attrib->GetAttributeType() == FbxNodeAttribute::eSkeleton;// && !((FbxSkeleton*) attrib)->GetSkeletonType() == FbxSkeleton::eLimbNode;

	return result;
}

void fbxLoader2::buildSkeletonCycle(FbxNode* boneNode, int parentBoneIndex)
{
	BoneData* bone1 = new BoneData();
	FbxNodeAttribute * attrib = boneNode->GetNodeAttribute();
	if(((FbxSkeleton*) attrib)->GetSkeletonType() == FbxSkeleton::eLimbNode)
	{
		bone1->SetName(boneNode->GetName());
		bone1->SetID(skeleton->GetBonesCount());
		bone1->SetParent(parentBoneIndex);

		skeleton->SetBones(bone1);
	}

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

void fbxLoader2::buildSkeleton(FbxNode *node)
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
				FbxSkeleton *skeletalMesh = node->GetSkeleton();
				if( !skeletalMesh ) return;
				FbxNode* boneNode = skeletalMesh->GetNode();
				buildSkeletonCycle(boneNode, -1);
					//rootbone1 = true;
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

FbxNode* fbxLoader2::findSkeletonRootBone(FbxNode* node)
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


void fbxLoader2::computeMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pGlobalPosition, FbxPose* pPose, int frame)
{
    FbxMesh* lMesh = pNode->GetMesh();
    const int lVertexCount = lMesh->GetControlPointsCount();

    // No vertex to draw.
    if (lVertexCount == 0)
    {
        return;
    }

    // If it has some defomer connection, update the vertices position
    const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
        (static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->IsActive();
    const bool lHasShape = lMesh->GetShapeCount() > 0;
    const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

    if (lHasDeformation)
    {
        //we need to get the number of clusters
        const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
        int lClusterCount = 0;
        for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
        {
            lClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
        }
        if (lClusterCount)
        {
            // Deform the vertex array with the skin deformer.
            ComputeSkinDeformation(pGlobalPosition, lMesh, pTime, pPose, frame);
        }
    }
}

//Compute the transform matrix that the cluster will transform the vertex.
void fbxLoader2::ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxCluster* pCluster,  FbxAMatrix& pVertexTransformMatrix, FbxTime pTime, FbxPose* pPose, int frame)
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
		lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = pGlobalPosition;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = GetGeometry(pCluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
			lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = pGlobalPosition; // bind pose matrix
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

// Deform the vertex array in classic linear way.
void fbxLoader2::ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh,  FbxTime& pTime, FbxPose* pPose, int frame)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
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

			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose, frame);

			//lVertexTransformMatrix.Transpose();
			FbxAMatrix identityM;
			identityM.SetIdentity();

			FbxVector4 rotation = lVertexTransformMatrix.GetROnly();
			FbxVector4 translation = lVertexTransformMatrix.GetT();
			FbxVector4 scaling = lVertexTransformMatrix.GetS();

			//rotation = FbxVector4(rotation.mData[0], rotation.mData[1], rotation.mData[2], rotation.mData[3]);
			//translation = FbxVector4 (translation.mData[0], translation.mData[1], translation.mData[2], translation.mData[3]);
			//scaling = FbxVector4 (scaling.mData[0], scaling.mData[1], scaling.mData[2], scaling.mData[3]);

			//lVertexTransformMatrix = FbxAMatrix(translation, rotation, scaling);			
			//lVertexTransformMatrix = FbxAMatrix(translation, rotation, scaling);	


			identityM = lVertexTransformMatrix * identityM;

			D3DXMATRIX convert = D3DXMATRIX(1,0,0,0,
											0,0,1,0,
											0,1,0,0,
											0,0,0,1);

			D3DXMATRIX setMatrix = D3DXMATRIX(	(float)identityM.mData[0].mData[0], (float)identityM.mData[1].mData[0], (float)identityM.mData[2].mData[0], (float)identityM.mData[3].mData[0],
												(float)identityM.mData[0].mData[1], (float)identityM.mData[1].mData[1], (float)identityM.mData[2].mData[1], (float)identityM.mData[3].mData[1],
												(float)identityM.mData[0].mData[2], (float)identityM.mData[1].mData[2], (float)identityM.mData[2].mData[2], (float)identityM.mData[3].mData[2],
												(float)identityM.mData[0].mData[3], (float)identityM.mData[1].mData[3], (float)identityM.mData[2].mData[3],1);

			//setMatrix *=0.5f;

			setMatrix = D3DXMATRIX(	(float)identityM.mData[0].mData[0], (float)identityM.mData[1].mData[0], (float)identityM.mData[2].mData[0], (float)identityM.mData[3].mData[0],
									(float)identityM.mData[0].mData[1], (float)identityM.mData[1].mData[1], (float)identityM.mData[2].mData[1], (float)identityM.mData[3].mData[1],
									//(float)identityM.mData[0].mData[2], (float)identityM.mData[1].mData[2], (float)identityM.mData[2].mData[2], (float)identityM.mData[3].mData[1],
									(float)identityM.mData[0].mData[2], (float)identityM.mData[1].mData[2], (float)identityM.mData[2].mData[2], (float)identityM.mData[3].mData[2],
									(float)identityM.mData[0].mData[3], (float)identityM.mData[1].mData[3], (float)identityM.mData[2].mData[3], 1);

			//setMatrix = setMatrix*convert;

			///////// juz prawie dziala. sprawdz jeszcze te addytywne itp.
			/// generalnie dodaj to do włosów i dorzuć poprzednią macierz, żeby liczyć przesunięcia.


			//setMatrix /= 2.54f; //skala jedna jest w cm, druga w inchach, nieważne czy zmieniam system skali ręcznie... bzdurka fbxa
			

			std::string nametype = lCluster->GetLink()->GetName();	

			animationStructure->GetSkeleton(frame)->GetBone(animationStructure->GetSkeleton(frame)->GetBoneByName(lCluster->GetLink()->GetName()))->SetTransformation(setMatrix);
		}//lClusterCount
	}

	delete [] lClusterDeformation;
	delete [] lClusterWeight;
}

// Deform the vertex array in Dual Quaternion Skinning way.
void fbxLoader2::ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxPose* pPose, int frame)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

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
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose, frame);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
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

	delete [] lDQClusterDeformation;
	delete [] lClusterWeight;
}

// Deform the vertex array according to the links contained in the mesh and the skinning type.
void fbxLoader2::ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxPose* pPose, int frame)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pPose, frame);
	}
	else if(lSkinningType == FbxSkin::eDualQuaternion)
	{
		int i = 0;
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pPose, frame);
	}
	else if(lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = pMesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pPose, frame);
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pPose, frame);
	}
}

FbxAMatrix fbxLoader2::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
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

FbxAMatrix fbxLoader2::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

FbxAMatrix fbxLoader2::GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}
