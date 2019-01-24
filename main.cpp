//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//	   - modified by FLL
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <time.h>
#include <math.h>
#include <ctime>

#include "Object.h"
#include "Terrain.h"

#define Win_WIDTH 1920*0.75//640
#define Win_HEIGHT 1080*0.75//480

float deltaTime = 0;

Object sword;
Terrain terrain;

struct WorldViewPerspectiveMatrix {
	XMMATRIX mWorld,mInvTraWorld,mView,mPerspective;
};
struct LightData {
	XMFLOAT4 pos = XMFLOAT4(0,2,0,0);
	XMFLOAT4 color = XMFLOAT4(1,1,1,1);//.a is intensity
} light;
XMFLOAT3 cameraPosition = XMFLOAT3(0,2,-3);
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
		if (gVertexLayout != nullptr)gVertexLayout->Release();							//MEMORY LEAKS
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
	data.pSysMem = &light;

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
	gDeviceContext->GSSetConstantBuffers(0, 1, &gMatrixBuffer);
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

	XMFLOAT3 at(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	XMMATRIX mView = XMMatrixLookAtLH(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&at), XMLoadFloat3(&up));

	XMMATRIX mPerspective = XMMatrixPerspectiveFovLH(XM_PI*0.45, (float)(Win_WIDTH) / (Win_HEIGHT), 0.1, 200);

	WorldViewPerspectiveMatrix mat;
	mat.mWorld = XMMatrixTranspose(worldMat);
	mat.mInvTraWorld = XMMatrixTranspose(worldMat.Invert().Transpose());
	mat.mView = XMMatrixTranspose(mView);
	mat.mPerspective = XMMatrixTranspose(mPerspective);

	gDeviceContext->UpdateSubresource(gMatrixBuffer, 0, 0, &mat, 0, 0);
}

void Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0.3, 0.3, 0.3, 1 };

	// use DeviceContext to talk to the API
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH,1,0);
	//objects
	shader_object.bindShadersAndLayout();
	int count = 5;
	for (int i = 0; i < count; i++)
	{
		float r = (6.28 / count)*i;
		float l = 1;
		sword.setPosition(float3(cos(rotation+r)*l,0.2,sin(rotation+r)*l));
		sword.rotateY(deltaTime*XM_2PI*0.25*(1.0f / 4));
		updateMatrixBuffer(sword.getWorldMatrix());
		sword.draw();
	}
	//terrain
	terrain.rotateY(deltaTime*XM_2PI*0.25*(1.0f / 4));
	shader_terrain.bindShadersAndLayout();
	updateMatrixBuffer(terrain.getWorldMatrix());
	terrain.draw();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		CreateCameraBuffer();

		CreateLightBuffer();

		CreateMatrixDataBuffer();

		sword.loadMesh("Meshes/Sword");
		sword.setScale(float3(0.1,0.1,0.1));

		terrain.create(XMINT2(100,100),4,2,L"Images/heightMap1.jpg");

		shader_object.createShaders(L"Effects/Vertex.hlsl", L"Effects/Geometry.hlsl", L"Effects/Fragment.hlsl");
		shader_terrain.createShaders(L"Effects/Vertex_Terrain.hlsl", L"Effects/Geometry_Terrain.hlsl", L"Effects/Fragment_Terrain.hlsl");

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			LARGE_INTEGER preTime;
			QueryPerformanceCounter(&preTime);
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				//rotate plane
				rotation += deltaTime*XM_2PI*0.25*(1.0f/4);
				//update lightdata buffer
				gDeviceContext->UpdateSubresource(gLightBuffer, 0, 0, &light, 0, 0);
				//update cameradata buffer
				XMFLOAT4 cpD = XMFLOAT4(cameraPosition.x,cameraPosition.y,cameraPosition.z,1);
				gDeviceContext->UpdateSubresource(gCameraBuffer, 0, 0, &cpD, 0, 0);

				Render(); //8. Rendera

				gSwapChain->Present(0, 0); //9. Växla front- och back-buffer
			}
			LARGE_INTEGER postTime;
			QueryPerformanceCounter(&postTime);
			deltaTime = 2*(postTime.QuadPart - preTime.QuadPart)/pow(10,7);
		}

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