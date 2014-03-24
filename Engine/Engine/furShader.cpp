#include "furShader.h"


furShader::furShader(void)
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_samplerState = 0;
	m_furBuffer = 0;
	m_animBuffer = 0;
	m_cameraBuffer = 0;
	m_lightBuffer = 0;
	m_modBuffer = 0;

	srand (time(NULL));
	diff = 0;
	counter = 0.0f;
	random1 = 0; 
	random2 = 0; 
	random3 = 0;
	randomTarget1 = 0;
	randomTarget2 = 0;
	randomTarget3 = 0;
	movement1 = 0;
	movement2 = 0;
	movement3 = 0; 
}


furShader::~furShader(void)
{
}


bool furShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"../Engine/fur.vs", L"../Engine/fur.ps");
	if(!result)
	{
		return false;
	}

	return true;
}

void furShader::Shutdown()
{
	ShutdownShader();

	return;
}

bool furShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float hairLength, D3DXMATRIX *boneMatrices, D3DXMATRIX *previousBones, int boneNumber, D3DXVECTOR3 cameraPosition, D3DXVECTOR3 lightDirection, D3DXVECTOR3 cam,  float windStrength, float gravityStrength, float dampeningStrength, float stiffnessModifier, float reflectModifier)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, hairLength, boneMatrices, previousBones, boneNumber, cameraPosition, lightDirection, cam, windStrength, gravityStrength, dampeningStrength, stiffnessModifier, reflectModifier);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool furShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;

	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[6];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerStateDesc;
	D3D11_SAMPLER_DESC samplerStateDesc1;
	D3D11_BUFFER_DESC furBufferDesc;
	D3D11_BUFFER_DESC animBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC modBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "VS_FurShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "FurShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "NORMAL";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	//jakos musze przekazac tablice
	polygonLayout[2].SemanticName = "BONES";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_UINT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "WEIGHTS";
	polygonLayout[3].SemanticIndex = 0;
	//DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "HAIRLENGTH";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;
	
	polygonLayout[5].SemanticName = "TEXCOORD";
	polygonLayout[5].SemanticIndex = 0;
	polygonLayout[5].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[5].InputSlot = 0;
	polygonLayout[5].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[5].InstanceDataStepRate = 0;


	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.MipLODBias = 0.0f;
	samplerStateDesc.MaxAnisotropy = 1;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerStateDesc.BorderColor[0] = 0;
	samplerStateDesc.BorderColor[1] = 0;
	samplerStateDesc.BorderColor[2] = 0;
	samplerStateDesc.BorderColor[3] = 0;
	samplerStateDesc.MinLOD = 0;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerStateDesc, &m_samplerState);
	if(FAILED(result))
	{
		return false;
	}

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	furBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	furBufferDesc.ByteWidth = sizeof(FurBufferType);
    furBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    furBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    furBufferDesc.MiscFlags = 0;
	furBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&furBufferDesc, NULL, &m_furBuffer);
	if(FAILED(result))
	{
		return false;
	}

	
	animBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	animBufferDesc.ByteWidth = sizeof(AnimationBufferType);
    animBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    animBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    animBufferDesc.MiscFlags = 0;
	animBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&animBufferDesc, NULL, &m_animBuffer);
	if(FAILED(result))
	{
		return false;
	}

	modBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	modBufferDesc.ByteWidth = sizeof(ModifiedBufferType);
    modBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    modBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    modBufferDesc.MiscFlags = 0;
	modBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&modBufferDesc, NULL, &m_modBuffer);
	if(FAILED(result))
	{
		return false;
	}

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if(FAILED(result))
	{
		return false;
	}

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void furShader::ShutdownShader()
{
	if(m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	if(m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = 0;
	}

	if(m_modBuffer)
	{
		m_modBuffer->Release();
		m_modBuffer = 0;
	}

	if(m_furBuffer)
	{
		m_furBuffer->Release();
		m_furBuffer = 0;
	}

	if(m_animBuffer)
	{
		m_animBuffer->Release();
		m_animBuffer = 0;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	if(m_samplerState)
	{
		m_samplerState->Release();
		m_samplerState = 0;
	}

	return;
}

void furShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error1.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool furShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float hairLength, D3DXMATRIX *boneMatrices, D3DXMATRIX *previousBones, int boneNumber, D3DXVECTOR3 cameraPosition, D3DXVECTOR3 lightDirection, D3DXVECTOR3 cam, float windStrength, float gravityStrength, float dampeningStrength, float stiffnessModifier, float reflectModifier)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	FurBufferType* dataPtr2;
	ModifiedBufferType* dataPtr3;
	LightBufferType* dataPtr4;
	CameraBufferType* dataPtr5;

	D3DXMATRIX ident = D3DXMATRIX();
	D3DXMatrixIdentity(&ident);
	diff = rand()%30-15;

	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->identity = ident;
	dataPtr->projection = projectionMatrix;
	for (int i = 0; i<boneNumber; i++)
	{
		dataPtr->bonesTranslation[i] = boneMatrices[i];
	}
	for (int i = 0; i<boneNumber; i++)
	{
		dataPtr->previousTranslation[i] = previousBones[i];
	}
	dataPtr->camera = cam;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	// Set furlength buffer
	result = deviceContext->Map(m_furBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}


	if (random1 >= randomTarget1-movement1-0.5f && random1 <= randomTarget1+movement1+0.5f)
	{
		randomTarget1 = rand()%52-26.0f;
		randomTarget1 *= windStrength;
		movement1 = abs(randomTarget1-random1)/((rand()%800+600));
	}
	if (random2 >= randomTarget2-movement2-0.5f && random2 <= randomTarget2+movement2+0.5f)
	{
		randomTarget2 = rand()%10-5.0f;
		randomTarget2 *= windStrength;
		movement2 = abs(randomTarget2-random2)/((rand()%500+200));
	}
	if (random3 >= randomTarget3-movement3-0.5f && random3 <= randomTarget3+movement3+0.5f)
	{
		randomTarget3 = rand()%40-20.0f;
		randomTarget3 *= windStrength;
		movement3 = abs(randomTarget3-random3)/((rand()%900+700));
	}
	if(random1 < randomTarget1)
	{
		random1 += movement1;
	}
	if(random1 > randomTarget1)
	{
		random1 -= movement1;
	}
	if(random2 < randomTarget2)
	{
		random2 += movement2;
	}
	if(random2 > randomTarget2)
	{
		random2 -= movement2;
	}
	if(random3 < randomTarget3)
	{
		random3 += movement3;
	}
	if(random3 > randomTarget3)
	{
		random3 -= movement3;
	}


	dataPtr2 = (FurBufferType*)mappedResource.pData;
	dataPtr2->length = hairLength;
	dataPtr2->wind = D3DXVECTOR3(0.0f+random1, 1.0f+random3, 0.0f);
	dataPtr2->time = diff;
	dataPtr2->filler = D3DXVECTOR3(0,0,0);


	deviceContext->Unmap(m_furBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_furBuffer);


	//camera buffer
	result = deviceContext->Map(m_modBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	dataPtr3 = (ModifiedBufferType*)mappedResource.pData;

	dataPtr3->dampeningAmp = dampeningStrength;
	dataPtr3->gravityStrength = gravityStrength;
	dataPtr3->stiffnessAmp = stiffnessModifier;
	dataPtr3->modpadding = 0.0f;

	deviceContext->Unmap(m_modBuffer, 0);

	bufferNumber = 2;

	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_modBuffer);



	//camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	dataPtr5 = (CameraBufferType*)mappedResource.pData;

	dataPtr5->cameraPos = cameraPosition;
	dataPtr5->reflectionMod = reflectModifier;

	deviceContext->Unmap(m_cameraBuffer, 0);

	bufferNumber = 0;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

	//lightbuffer
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	dataPtr4 = (LightBufferType*)mappedResource.pData;

	dataPtr4->ambientColor = D3DXVECTOR4(0,0,0,0);
	dataPtr4->diffuseColor = D3DXVECTOR4(0,0,0,0);
	dataPtr4->lightDirection = lightDirection;
	dataPtr4->specularColor = D3DXVECTOR4(1,1,1,1);
	dataPtr4->specularPower = 10;
	
	deviceContext->Unmap(m_lightBuffer, 0);

	bufferNumber = 1;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	deviceContext->VSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

void furShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//deviceContext->VSSetSamplers(0, 1, &m_samplerState);
	deviceContext->PSSetSamplers(0, 1, &m_samplerState);

	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}