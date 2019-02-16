//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//	   - modified by FLL
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <iostream>
#include <time.h>
#include <math.h>

#include <Keyboard.h>
#include <Mouse.h>

#include "Object.h"
#include "Terrain.h"

#define Win_WIDTH 1920*0.75//640
#define Win_HEIGHT 1080*0.75//480

#define DEF_BUFFERCOUNT 3

float deltaTime = 0;

Array<Mesh> meshes;
Array<Object> objects;
Object sphere;
Object cube;

Terrain terrain;



//mouse Picking location
float3 lookPos(0, 0, 0);
//input stuff
std::unique_ptr<DirectX::Keyboard> m_keyboard;
std::unique_ptr<Mouse> mouse = std::make_unique<Mouse>();
float2 mousePos;
Mouse::ButtonStateTracker mouseTracker;
//world data
struct WorldViewPerspectiveMatrix {
	XMMATRIX mWorld,mInvTraWorld,mWorldViewPerspective;
};
struct LightData {
	const float4 lightCount = float4(10, 0, 0,0);
	float4 pos[10];
	float4 color[10];//.a is intensity
} lights;



//player variables
bool grounded = false;
float gravityForce = 3;
float3 gravityDirection = float3(0,-1,0);
float3 acceleration = float3(0,0,0);
float3 velocity = float3(0,0,0);
float3 cameraPosition = float3(0,3,0);
float3 cameraForward = float3(0,-1,0);
float2 cameraRotation = float2(0,0);
float rotation = 0;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);

// Most directX Objects are COM Interfaces
// https://es.wikipedia.org/wiki/Component_Object_Model
IDXGISwapChain* gSwapChain = nullptr;

// Device and DeviceContext are the most common objects to
// instruct the API what to do. It is handy to have a reference
// to them available for simple applications.
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
// A "view" of a particular resource (the color buffer)
ID3D11RenderTargetView* gBackbufferRTV = nullptr;
//depth buffer
ID3D11DepthStencilView* gDepthStencilView = nullptr;
// a resource to store the matrix in the GPU
ID3D11Buffer* gMatrixBuffer = nullptr;
ID3D11Buffer* gLightBuffer = nullptr;
ID3D11Buffer* gCameraBuffer = nullptr;

struct ShaderSet {
protected:
	ID3D11InputLayout* gVertexLayout = nullptr;

	ID3D11VertexShader* gVertexShader = nullptr;
	ID3D11GeometryShader* gGeometryShader = nullptr;
	ID3D11PixelShader* gPixelShader = nullptr;
	ID3DBlob* createVertexShader(LPCWSTR filename) {
		ID3DBlob* pVS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		HRESULT result = D3DCompileFromFile(
			filename, // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"VS_main",		// entry point
			"vs_5_0",		// shader model (target)
			D3DCOMPILE_DEBUG,	// shader compile options (DEBUGGING)
			0,				// IGNORE...DEPRECATED.
			&pVS,			// double pointer to ID3DBlob		
			&errorBlob		// pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pVS)
				pVS->Release();
			//return result;
		}

		gDevice->CreateVertexShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			&gVertexShader
		);
		return pVS;
	}
	HRESULT createGeometryShader(LPCWSTR filename) {
		ID3DBlob* pGS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT result = D3DCompileFromFile(
			filename,
			nullptr,
			nullptr,
			"GS_main",
			"gs_5_0",
			D3DCOMPILE_DEBUG,
			0,
			&pGS,
			&errorBlob
		);

		// compilation failed?
		if (FAILED(result))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pGS)
				pGS->Release();
			return result;
		}

		gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);

		pGS->Release();
	}
	HRESULT createFragmentShader(LPCWSTR filename) {
		ID3DBlob* pPS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT result = D3DCompileFromFile(
			filename, // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"PS_main",		// entry point
			"ps_5_0",		// shader model (target)
			D3DCOMPILE_DEBUG,	// shader compile options
			0,				// effect compile options
			&pPS,			// double pointer to ID3DBlob		
			&errorBlob			// pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pPS)
				pPS->Release();
			return result;
		}

		gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
		// we do not need anymore this COM object, so we release it.
		pPS->Release();
	}
public:
	bool createShaders(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0) {
		ID3DBlob* pVS = createVertexShader(vertexName);
		if (inputDesc == nullptr) {
			D3D11_INPUT_ELEMENT_DESC standardInputDesc[] = {
				{
					"Position",		// "semantic" name in shader
					0,				// "semantic" index (not used)
					DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
					0,							 // input slot
					0,							 // offset of first element
					D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
					0							 // used for INSTANCING (ignore)
				},
				{
					"TexCoordinate",
					0,
					DXGI_FORMAT_R32G32_FLOAT,
					0,
					12,
					D3D11_INPUT_PER_VERTEX_DATA,
					0
				},
				{
					"Normal",
					0,
					DXGI_FORMAT_R32G32B32_FLOAT,
					0,
					20,
					D3D11_INPUT_PER_VERTEX_DATA,
					0
				}
			};
			gDevice->CreateInputLayout(standardInputDesc, ARRAYSIZE(standardInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
		}
		else {
			gDevice->CreateInputLayout(inputDesc, inputDescCount, pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
			delete[] inputDesc;
		}
		pVS->Release();

		createGeometryShader(geometryName);
		createFragmentShader(fragmentName);
		return true;
	}
	void bindShadersAndLayout() {
		gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
		gDeviceContext->HSSetShader(nullptr, nullptr, 0);
		gDeviceContext->DSSetShader(nullptr, nullptr, 0);
		gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
		gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

		gDeviceContext->IASetInputLayout(gVertexLayout);
	}
	void release() {
		if (gVertexShader != nullptr)gVertexShader->Release();
		if (gGeometryShader != nullptr)gGeometryShader->Release();
		if (gPixelShader != nullptr)gPixelShader->Release();
		if (gVertexLayout != nullptr)gVertexLayout->Release();
	}
	ShaderSet(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0){
		createShaders(vertexName,geometryName,fragmentName,inputDesc,inputDescCount);
	}
	ShaderSet() {

	}
	~ShaderSet() {
		release();
	}
	
};
ShaderSet shader_object;
ShaderSet shader_terrain;
ShaderSet shader_object_onlyMesh;
ShaderSet gShader_Deferred;

//Deffered shading
struct RenderTarget {
	ID3D11RenderTargetView		*renderTragetVeiw			= nullptr;
	ID3D11Texture2D				*texture					= nullptr;
	ID3D11ShaderResourceView	*shaderResourceVeiw			= nullptr;
};

struct FullScreenQuad {
	Vertex corners[4];
	ID3D11Buffer *FSQVertexBuffer = nullptr;
	ID3D11InputLayout *FSQInputLayout = nullptr;
	
	void createFSQ() {
		corners[0] = Vertex(float3(Win_WIDTH / 2 * -1, Win_HEIGHT / 2, 0), float2(0, 0), float3(0, 0, -1));
		corners[1] = Vertex(float3(Win_WIDTH / 2 , Win_HEIGHT / 2, 0), float2(1, 0), float3(0, 0, -1));
		corners[2] = Vertex(float3(Win_WIDTH / 2 * -1, Win_HEIGHT / 2*-1, 0), float2(0, 1), float3(0, 0, -1));
		corners[3] = Vertex(float3(Win_WIDTH / 2 , Win_HEIGHT / 2*-1, 0), float2(1, 1), float3(0, 0, -1));

		D3D11_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(bufDesc));

		bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		bufDesc.Usage = D3D11_USAGE_DEFAULT;

		bufDesc.ByteWidth = sizeof(corners);

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = corners;

		gDevice->CreateBuffer(&bufDesc, &data, &FSQVertexBuffer);
		
	}
	void killItWithFire() {
		FSQInputLayout->Release();
		FSQVertexBuffer->Release();
	}
};

RenderTarget geometryBuffer[DEF_BUFFERCOUNT];
FullScreenQuad gFSQ;


bool createGBuffer() // Om denna flyttas till en egen klass senare beh�ver den tillg�ng till Device och sk�rm info(width,height,depth
{
	HRESULT result;

	// Initialize the render target texture description.
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = Win_WIDTH;
	textureDesc.Height = Win_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target textures.
	for (int i = 0; i < DEF_BUFFERCOUNT; i++)
	{
		result = gDevice->CreateTexture2D(&textureDesc, NULL, &geometryBuffer[i].texture);
		if (FAILED(result))
		{
			return false;
		}
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target views.
	for (int i = 0; i < DEF_BUFFERCOUNT; i++)
	{
		result = gDevice->CreateRenderTargetView(geometryBuffer[i].texture, &renderTargetViewDesc, &geometryBuffer[i].renderTragetVeiw);
		if (FAILED(result))
		{
			return false;
		}
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource views.
	for (int i = 0; i < DEF_BUFFERCOUNT; i++)
	{
		result = gDevice->CreateShaderResourceView(geometryBuffer[i].texture, &shaderResourceViewDesc, &geometryBuffer[i].shaderResourceVeiw);
		if (FAILED(result))
		{
			return false;
		}
	}

	//// Initialize the description of the depth buffer.
	//D3D11_TEXTURE2D_DESC depthBufferDesc;
	//ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	//// Set up the description of the depth buffer.
	//depthBufferDesc.Width = Win_WIDTH;
	//depthBufferDesc.Height = Win_HEIGHT;
	//depthBufferDesc.MipLevels = 1;
	//depthBufferDesc.ArraySize = 1;
	//depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthBufferDesc.SampleDesc.Count = 1;
	//depthBufferDesc.SampleDesc.Quality = 0;
	//depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//depthBufferDesc.CPUAccessFlags = 0;
	//depthBufferDesc.MiscFlags = 0;

	//// Create the texture for the depth buffer using the filled out description.
	//result = gDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	//// Initailze the depth stencil view description.
	//ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//// Set up the depth stencil view description.
	//D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	//depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//depthStencilViewDesc.Texture2D.MipSlice = 0;

	//// Create the depth stencil view.
	//result = device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	//// Setup the viewport for rendering.
	//m_viewport.Width = (float)textureWidth;
	//m_viewport.Height = (float)textureHeight;
	//m_viewport.MinDepth = 0.0f;
	//m_viewport.MaxDepth = 1.0f;
	//m_viewport.TopLeftX = 0.0f;
	//m_viewport.TopLeftY = 0.0f;

	return true;
}


void BindFirstPass();
void BindSecondPass();

void CreateLightBuffer() {
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	// what type of buffer will this be?
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// what type of usage (press F1, read the docs)
	desc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	desc.ByteWidth = sizeof(LightData);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &lights;

	gDevice->CreateBuffer(&desc, &data, &gLightBuffer);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gLightBuffer);
}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)Win_WIDTH;
	vp.Height = (float)Win_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void CreateMatrixDataBuffer() {

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	// what type of buffer will this be?
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// what type of usage (press F1, read the docs)
	desc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	desc.ByteWidth = sizeof(WorldViewPerspectiveMatrix);

	gDevice->CreateBuffer(&desc,nullptr,&gMatrixBuffer);
	gDeviceContext->VSSetConstantBuffers(0, 1, &gMatrixBuffer);
}

void CreateCameraBuffer() {

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	// what type of buffer will this be?
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// what type of usage (press F1, read the docs)
	desc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	desc.ByteWidth = sizeof(XMFLOAT4);

	gDevice->CreateBuffer(&desc, nullptr, &gCameraBuffer);
	gDeviceContext->PSSetConstantBuffers(1, 1, &gCameraBuffer);
}

void updateMatrixBuffer(float4x4 worldMat) {
	XMFLOAT3 at = cameraPosition+cameraForward;
	XMFLOAT3 up(0, 1, 0);
	XMMATRIX mView = XMMatrixLookAtLH(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&at), XMLoadFloat3(&up));

	XMMATRIX mPerspective = XMMatrixPerspectiveFovLH(XM_PI*0.45, (float)(Win_WIDTH) / (Win_HEIGHT), 0.01, 200);

	WorldViewPerspectiveMatrix mat;
	mat.mWorld = XMMatrixTranspose(worldMat);
	mat.mInvTraWorld = XMMatrixTranspose(worldMat.Invert().Transpose());
	mat.mWorldViewPerspective = XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(worldMat,mView),mPerspective));

	gDeviceContext->UpdateSubresource(gMatrixBuffer, 0, 0, &mat, 0, 0);
}

void mousePicking(float screenSpace_x, float screenSpace_y) {
	//pos between -1 and 1
	float SSxN = 2 * (screenSpace_x / (Win_WIDTH)) - 1;
	float SSyN = -(2 * (screenSpace_y / (Win_HEIGHT)) - 1);
	float4 vRayPos(0,0,0,1);
	float4 vRayDir(SSxN,SSyN,1,0);
	vRayDir.Normalize();
	//convert to world space
	XMFLOAT3 at = cameraPosition + cameraForward;
	XMFLOAT3 up(0, 1, 0);
	float4x4 mInvView = ((float4x4)XMMatrixLookAtLH(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&at), XMLoadFloat3(&up))).Invert();
	float4 wRayPos = XMVector4Transform(vRayPos, mInvView);
	float4 wRayDir = XMVector4Transform(vRayDir, mInvView);
	//check all objects
	float t = -1;
	for (int i = 0; i < objects.length(); i++)
	{
		float tt = objects[i].castRayOnObject(float3(wRayPos.x, wRayPos.y, wRayPos.z),float3(wRayDir.x, wRayDir.y, wRayDir.z));
		if ((tt < t && tt >= 0) || t < 0)t = tt;
	}
	//apply position
	if (t >= 0) {
		float4 target = (wRayPos + wRayDir * t);
		lookPos = float3(target.x, target.y, target.z);
	}
}

void drawBoundingBox(Object obj) {
	shader_object_onlyMesh.bindShadersAndLayout();
	cube.setRotation(obj.getRotation());
	cube.setPosition(obj.getBoundingBoxPos());
	cube.setScale(obj.getBoundingBoxSize());
	updateMatrixBuffer(cube.getWorldMatrix());
	cube.draw();
}

void RenderFSQ() {
	
	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);
	float clearColor[] = { 1,1,1,1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);

	gShader_Deferred.bindShadersAndLayout();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;

	gDeviceContext->IASetVertexBuffers(0, 1, &gFSQ.FSQVertexBuffer,&strides, &offset);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	gDeviceContext->IASetInputLayout(gFSQ.FSQInputLayout);

	gDeviceContext->PSSetConstantBuffers(0, 1, &gCameraBuffer);
	ID3D11ShaderResourceView* srvArray[] = {
		geometryBuffer[0].shaderResourceVeiw,
		geometryBuffer[1].shaderResourceVeiw,
		geometryBuffer[2].shaderResourceVeiw,
	};

	gDeviceContext->PSSetShaderResources(0, DEF_BUFFERCOUNT, srvArray);
	gDeviceContext->PSSetConstantBuffers(lights)

	gDeviceContext->Draw(4, 0);

}

void Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0.1, 0.1, 0.1, 1 };

	// use DeviceContext to talk to the API
	/*gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH,1,0);*/
	BindFirstPass();

	//objects
	shader_object.bindShadersAndLayout();
	for (int i = 0; i < objects.length(); i++)
	{
		updateMatrixBuffer(objects[i].getWorldMatrix());
		objects[i].draw();
	}
	//lights
	shader_object_onlyMesh.bindShadersAndLayout();
	for (int i = 0; i < lights.lightCount.x; i++)
	{
		sphere.setPosition(float3(lights.pos[i].x, lights.pos[i].y, lights.pos[i].z));
		sphere.setScale(float3(lights.color[i].w, lights.color[i].w, lights.color[i].w)*0.03);
		updateMatrixBuffer(sphere.getWorldMatrix());
		sphere.draw();
	}
	//mousePicking sphere
	shader_object_onlyMesh.bindShadersAndLayout();
	sphere.setScale(float3(1, 1, 1)*0.1);
	sphere.setPosition(lookPos);
	updateMatrixBuffer(sphere.getWorldMatrix());
	sphere.draw();

	sphere.setPosition(cameraPosition+float3(0,0,1));
	//sphere.setScale(float3(0.1, 0.1, 1));
	updateMatrixBuffer(sphere.getWorldMatrix());
	sphere.draw();
	sphere.setPosition(cameraPosition + float3(0, -1, 0));
	//sphere.setScale(float3(0.1, 1, 0.1));
	updateMatrixBuffer(sphere.getWorldMatrix());
	sphere.draw();
	sphere.setPosition(cameraPosition + float3(1, 0, 0));
	//sphere.setScale(float3(1, 0.1, 0.1));
	updateMatrixBuffer(sphere.getWorldMatrix());
	//sphere.draw();
	//terrain
	shader_terrain.bindShadersAndLayout();
	updateMatrixBuffer(terrain.getWorldMatrix());
	terrain.draw();
}


int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa f�nster
	
	mouse->SetWindow(wndHandle);
	mouse->SetVisible(false);

	//srand(time(NULL));
	m_keyboard = std::make_unique<Keyboard>();

	CoInitialize(nullptr);

	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. S�tt viewport

		CreateCameraBuffer();

		CreateLightBuffer();

		CreateMatrixDataBuffer();
		
		gFSQ.createFSQ();

		terrain.create(XMINT2(200, 200), 10, 5, L"Images/heightMap2.png", smoothShading);
		float3 sc = terrain.getTerrainSize();
		//lights
		for (int i = 0; i < lights.lightCount.x; i++)
		{
			float3 p = float3(0,2,0)+ terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().y / 2, terrain.getTerrainSize().y / 2));
			lights.pos[i] = float4(p.x,p.y,p.z,1);
			lights.color[i] = float4(random(0, 1), random(0, 1), random(0, 1), 0);
			lights.color[i].Normalize();
			lights.color[i].w = random(1, 2);
		}

		//meshes
		meshes.appendCapacity(100);//CANNOT COPY MESH OBJECT
		meshes.add(Mesh()); meshes[0].loadMesh("Meshes/Sword", flatShading);
		meshes.add(Mesh()); meshes[1].loadMesh("Meshes/sphere", smoothShading);
		meshes.add(Mesh()); meshes[2].loadMesh("Meshes/cube", flatShading);
		meshes.add(Mesh()); meshes[3].loadMesh("Meshes/tree1", smoothShading);

		objects.appendCapacity(1000);
		for (int i = 0; i < 10; i++)
		{
			Object swd = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().y / 2, terrain.getTerrainSize().y / 2)) ,
				float3(0,random(0,3.14*2),0), 
				float3(0.1,0.1,0.1), 
				&meshes[0]);
			objects.add(swd);
		}
		for (int i = 0; i < 1; i++)
		{
			Object tree = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().y / 2, terrain.getTerrainSize().y / 2)), 
				float3(0, random(0, 3.14 * 2), 0), 
				float3(1, 1, 1), 
				&meshes[3]
			);
			objects.add(tree);
		}

		sphere.giveMesh(&meshes[1]);
		cube.giveMesh(&meshes[2]);

		shader_object.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment.hlsl");
		shader_object_onlyMesh.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment_onlyMesh.hlsl");
		shader_terrain.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment_Terrain.hlsl");
		gShader_Deferred.createShaders(L"Effects/Vertex_Deferred.hlsl", nullptr, L"Effects/Fragment_Deferred.hlsl");

		ShowWindow(wndHandle, nCmdShow);

		clock_t time;
		while (WM_QUIT != msg.message)
		{
			time = clock();
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				//Mouse
				Mouse::State state = mouse->GetState();
				mouseTracker.Update(state);
				if (state.leftButton) {
					//do something every frame
				}
				if (mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED) {
					//do something once
				}
				//camera rotation with mouse
				RECT r; GetWindowRect(wndHandle, &r);//get window size

				POINT newPos;//get cursor position
				GetCursorPos(&newPos);

				float2 diff = float2(newPos.x, newPos.y) - mousePos;

				SetCursorPos((r.right + r.left) / 2, (r.bottom + r.top) / 2);//set cursor to middle of window
				GetCursorPos(&newPos);
				mousePos = float2(newPos.x,newPos.y);//save cursor position

				cameraRotation += float2(diff.y,diff.x)*0.002;//add mouse rotation

				if(mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED)mousePicking((float)Win_WIDTH/2,(float)Win_HEIGHT/2);
				//keyboard
				Keyboard::State kb = m_keyboard->GetState();
				//close window
				if (kb.Escape) {
					break;
				}
				//cameraRotation with arrow keys
				float rotSpeed = 2 * deltaTime;
				if (kb.Up) {
					cameraRotation.x -= rotSpeed;
				}
				if (kb.Down) {
					cameraRotation.x += rotSpeed;
				}
				if (kb.Left) {
					cameraRotation.y -= rotSpeed;
				}
				if (kb.Right) {
					cameraRotation.y += rotSpeed;
				}
				//clamp rotation
				if (cameraRotation.x > (3.14 / 2)*0.9) cameraRotation.x = (3.14 / 2)*0.9;
				if (cameraRotation.x < (-3.14 / 2)*0.9) cameraRotation.x = (-3.14 / 2)*0.9;
				//calc rotation matrix to use later
				float4x4 rotMat = float4x4::CreateRotationX(cameraRotation.x)*float4x4::CreateRotationY(cameraRotation.y);
				cameraForward = XMVector3Transform(float4(0,0,1,1),rotMat);
				//camera movement
				float speed = 1;
				float jumpForce = 2;
				float3 left = cameraForward.Cross(float3(0,1,0));
				float3 forward = XMVector3Transform(float4(0,0,1,1),float4x4::CreateRotationY(cameraRotation.y));
				left.Normalize();
				float3 movement(0, 0, 0);
				if (kb.LeftShift) speed *= 2;//sprint
				if (kb.W) {
					movement += forward * speed;
				}
				if (kb.S) {
					movement -= forward * speed;
				}
				if (kb.A) {
					movement += left * speed;
				}
				if (kb.D) {
					movement -= left * speed;
				}
				if (kb.Space && grounded) {//jump
					velocity.y = jumpForce;
					grounded = false;
				}
				if(!grounded)velocity += gravityDirection * gravityForce * deltaTime;//dont apply gravity if on ground
				cameraPosition += movement*deltaTime + velocity*deltaTime;
				//collision, only affects y-axis
				float3 nextPos = cameraPosition + float3(0,-1,0);
				float hy = terrain.getHeightOfTerrainFromCoordinates(nextPos.x, nextPos.z);
				if (nextPos.y < hy) {//if below terrain then add force up
					cameraPosition.y += (hy - nextPos.y) * deltaTime * 5;
					grounded = true;
					velocity = float3(0,0,0);
				}
				else grounded = false;

				//rotate
				rotation += deltaTime*XM_2PI*0.25*(1.0f/4);
				//update lightdata buffer
				gDeviceContext->UpdateSubresource(gLightBuffer, 0, 0, &lights, 0, 0);
				//update cameradata buffer
				XMFLOAT4 cpD = XMFLOAT4(cameraPosition.x,cameraPosition.y,cameraPosition.z,1);
				gDeviceContext->UpdateSubresource(gCameraBuffer, 0, 0, &cpD, 0, 0);

				Render(); //8. Rendera

				RenderFSQ();
				gSwapChain->Present(0, 0); //9. V�xla front- och back-buffer
			}
			time = clock() - time;
			deltaTime = (float)time/1000.0f;
		}

		CoUninitialize();

		gMatrixBuffer->Release();
		gLightBuffer->Release();

		gDepthStencilView->Release();
		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();
		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

void BindFirstPass()
{
	//gDeviceContext->IASetInputLayout();
	ID3D11RenderTargetView* renderTargets[] = {
		gBackbufferRTV,
		//geometryBuffer[0].renderTragetVeiw,
		geometryBuffer[1].renderTragetVeiw,
		geometryBuffer[2].renderTragetVeiw,
	};
	
	gDeviceContext->OMSetRenderTargets(DEF_BUFFERCOUNT, renderTargets, gDepthStencilView);
	//gDeviceContext->RSSetViewports(1,&)

	//clear rendertargets
	float colors[] = { 0,0,0,1.f };
	for(int i = 0; i < DEF_BUFFERCOUNT;i++)
	gDeviceContext->ClearRenderTargetView(renderTargets[i], colors);

	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, Win_WIDTH, Win_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;	
	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		//Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		D3D11_TEXTURE2D_DESC DeStDesc;
		DeStDesc.Width = Win_WIDTH;
		DeStDesc.Height = Win_HEIGHT;
		DeStDesc.ArraySize = 1;
		DeStDesc.MipLevels = 1;
		DeStDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		DeStDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		DeStDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		DeStDesc.CPUAccessFlags = 0;
		DeStDesc.MiscFlags = 0;
		DeStDesc.SampleDesc = scd.SampleDesc;//same as swapChain
		ID3D11Texture2D* tex = 0;
		gDevice->CreateTexture2D(&DeStDesc, 0, &tex);
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
		ZeroMemory(&viewDesc,sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		viewDesc.Format = DeStDesc.Format;
		viewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Flags = 0;
		viewDesc.Texture2D.MipSlice = 0;
		HRESULT hr2 = gDevice->CreateDepthStencilView(tex, &viewDesc, &gDepthStencilView);

		tex->Release();

		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, gDepthStencilView);
	}
	return hr;
}