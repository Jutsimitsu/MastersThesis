#include "meshRenderer.h"


meshRenderer::meshRenderer(void)
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	tex = 0;
}

meshRenderer::meshRenderer(fbxLoader2 &m_loader)
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	loader = m_loader;
	tex = 0;
}

meshRenderer::~meshRenderer(void)
{
	ShutdownBuffers();
}

ID3D11ShaderResourceView* meshRenderer::GetTexture()
{
	return tex->GetTexture();
}

void meshRenderer::RenderScene(ID3D11DeviceContext *context)
{
	RenderBuffers(context);
}

bool meshRenderer::Initialize(ID3D11Device* device, WCHAR* filename, D3DXVECTOR4 **texMap, int sizeX, int sizeY)
{
	bool result;

	result = InitializeBuffers(device, texMap, sizeX, sizeY);
	if(!result)
	{
		return false;
	}

	result = LoadTexture(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}

bool meshRenderer::InitializeBuffers(ID3D11Device* device, D3DXVECTOR4** texMap, int sizeX, int sizeY)
{
	vertexType* vertices;
	unsigned int* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;
	bool tests = false;
	float maxvalue = 0.0f;

	vertices = new vertexType[loader.getIndexMaxCount()];

	int b = loader.getIndexMaxCount();
	if(!vertices)
	{
		return false;
	}


	int a = loader.getIndexMaxCount();
	// Create the index array.
	indices = new unsigned int[a];
	if(!indices)
	{
		return false;
	}

	// Load the vertex array and index array with data.
	for(int i=0; i<loader.getIndexMaxCount(); i++)
	{
		vertices[i].position = D3DXVECTOR3(loader.getVertex(loader.getIndex(i)).x,loader.getVertex(loader.getIndex(i)).y, loader.getVertex(loader.getIndex(i)).z);	
		//vertices[i].position = D3DXVECTOR3(loader.getVertex(loader.getIndex(i)).x/ loader.getScale().x,loader.getVertex(loader.getIndex(i)).y /loader.getScale().y, loader.getVertex(loader.getIndex(i)).z/ loader.getScale().z);	
		vertices[i].normal = D3DXVECTOR3((loader.getIndices(i)).normal1.x ,(loader.getIndices(i)).normal1.y, (loader.getIndices(i)).normal1.z);	
		vertices[i].uv = D3DXVECTOR2((loader.getIndices(i)).uv1.x, (loader.getIndices(i)).uv1.y);
		float x = (loader.getIndices(i)).uv1.x;
		float y = (loader.getIndices(i)).uv1.y;
		vertices[i].hairLength = 255.0f-texMap[(int)(y*sizeX)][(int)(x*sizeY)].w;

		//loader.getWeightNumber(loader.getIndex(i))

		if (loader.getVertex(loader.getIndex(i)).y > maxvalue)
		{
			maxvalue = loader.getVertex(loader.getIndex(i)).y;
		}

		for (int j = 0; j<4; j++)
		{
			if (loader.getBoneWeight(loader.getIndex(i), j)>0 && loader.getBoneWeight(loader.getIndex(i), j)<loader.getSkeleton()->GetBonesCount()+1) // możliwe że tutaj powinno być +1
				vertices[i].bones[j] = (unsigned int)loader.getBoneWeight(loader.getIndex(i), j);
			else 
				vertices[i].bones[j] = 0;

			if (loader.getWeight(loader.getIndex(i), j)>0.0f && loader.getWeight(loader.getIndex(i), j) <= 1.0f)
				vertices[i].weights[j] = loader.getWeight(loader.getIndex(i), j);
			else 
				vertices[i].weights[j] = 0.0f;

			if (vertices[i].weights[j] > 1.0f)
			{
				int dafuq = 0;
			}
		}		
	}

	

	for(int j = 0; j< loader.getIndexMaxCount(); j++)
	{	
		indices[j] = j;
	}

	maxvalue;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(vertexType) * loader.getIndexMaxCount(); //*loader.getSkeleton()->GetBonesCount()*36; //* loader.getIndexMaxCount();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * loader.getIndexMaxCount();//*loader.getSkeleton()->GetBonesCount()*36; // loader.getIndexMaxCount();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void meshRenderer::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
	return;
}

void meshRenderer::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(vertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool meshRenderer::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	bool result;

	tex = new textureProcessor;
	if(!tex)
	{
		return false;
	}

	result = tex->InitializeGPU(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}

void meshRenderer::ReleaseTexture()
{
	if(tex)
	{
		tex->Shutdown();
		delete tex;
		tex = 0;
	}

	return;
}

void meshRenderer::ShutDown()
{
	ReleaseTexture();
	ShutdownBuffers();
	return;
}
