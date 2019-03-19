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
#include "LightManager.h"
#include "Deferred.h"
#include "QuadTree.h"
#include "TextureBlurrer.h"
#include "FrontToBack.h"

const int DEF_BUFFERCOUNT = 3;

float deltaTime = 0;

Array<Mesh> meshes;
Array<Object> objects;
Object sphere;
Object cube;

Terrain terrain;

LightManager lightManager;

//mouse Picking location
float3 lookPos(0, 0, 0);
//input stuff
std::unique_ptr<DirectX::Keyboard> m_keyboard;
std::unique_ptr<Mouse> mouse = std::make_unique<Mouse>();
float2 mousePos;
Mouse::ButtonStateTracker mouseTracker;
//world data
struct WorldViewPerspectiveMatrix {
	XMMATRIX mWorld, mInvTraWorld, mWorldViewPerspective;
};
//player variables
bool grounded = false;
float gravityForce = 3;
float3 gravityDirection = float3(0, -1, 0);
float3 acceleration = float3(0, 0, 0);
float3 velocity = float3(0, 0, 0);
float3 cameraPosition = float3(1, 5, 1);
float3 cameraForward = float3(0, -1, 0);
float2 cameraRotation = float2(0, 0);
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
ID3D11Buffer* gCameraBuffer = nullptr;

ShaderSet shader_object;
ShaderSet shader_terrain;
ShaderSet shader_object_onlyMesh;
ShaderSet gShader_Deferred;

////Deffered shading

Deferred gDeferred;

//GaussianBlurring
TextureBlurrer textureBlurrer;

// Viewdata for use in setting view matrix and view frustum
struct ViewData {
	XMFLOAT3 up;
	float fowAngle, aspectRatio, nearZ, farZ;

	ViewData()
	{
		up = XMFLOAT3(0, 1, 0);
		fowAngle = XM_PI * 0.45;
		aspectRatio = (float)(Win_WIDTH) / (Win_HEIGHT);
		nearZ = 0.01;
		farZ = 50;
	}
}viewData;

QuadTree gQuadTree(float3(0, 3, 0), float3(10, 3, 10), 3);
Array<int> gIndexArray;  // Visible objects
// Bools to showcase effects.
bool gFirstPerson = true;
bool gFirstPersonPressed = false;
bool gShowFrustum = false;
bool gShowFrustumPressed = false;
bool gShowFrontToBack = false;
bool gShowFrontToBackPressed = false;

void SetViewport(float width, float height)
{
	D3D11_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void SetViewportSM()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)SMAP_WIDTH;
	vp.Height = (float)SMAP_HEIGHT;
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

	gDevice->CreateBuffer(&desc, nullptr, &gMatrixBuffer);
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

void updateMatrixBuffer(float4x4 worldMat) { // Lägg till så camPos o camForward är parametrar
	XMFLOAT3 at = cameraPosition + cameraForward;
	//XMFLOAT3 up(0, 1, 0);

	bool povPlayer = gFirstPerson;
	XMMATRIX view;
	if(povPlayer) view = XMMatrixLookAtLH(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&at), XMLoadFloat3(&viewData.up));
	else
	{
		XMFLOAT3 camPos = float3(8, 10, 0);
		at = float3(0, 0, 0);
		view = XMMatrixLookAtLH(XMLoadFloat3(&camPos), XMLoadFloat3(&at), XMLoadFloat3(&viewData.up));
	}
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(viewData.fowAngle, viewData.aspectRatio, viewData.nearZ, viewData.farZ);

	WorldViewPerspectiveMatrix mat;
	mat.mWorld = XMMatrixTranspose(worldMat);
	mat.mInvTraWorld = XMMatrixTranspose(worldMat.Invert().Transpose());
	mat.mWorldViewPerspective = XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(worldMat, view), perspective));

	gDeviceContext->UpdateSubresource(gMatrixBuffer, 0, 0, &mat, 0, 0);
}

void mousePicking(float screenSpace_x, float screenSpace_y) {
	//pos between -1 and 1
	float SSxN = 2 * (screenSpace_x / (Win_WIDTH)) - 1;
	float SSyN = -(2 * (screenSpace_y / (Win_HEIGHT)) - 1);
	float4 vRayPos(0, 0, 0, 1);
	float4 vRayDir(SSxN, SSyN, 1, 0);
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
		float tt = objects[i].castRayOnObject(float3(wRayPos.x, wRayPos.y, wRayPos.z), float3(wRayDir.x, wRayDir.y, wRayDir.z));
		if ((tt < t && tt > 0) || t < 0)t = tt;
	}
	//apply position
	if (t > 0) {
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

void drawToShadowMap() {
	for (int iLight = 0; iLight < lightManager.lightCount(); iLight++)
	{
		lightManager.setShadowMapForDrawing(iLight);
		//objects		
		for (int iObj = 0; iObj < objects.length(); iObj++)
		{
			lightManager.updateMatrixBuffer(objects[iObj].getWorldMatrix(), iLight);
			objects[iObj].draw();
		}

		// If shadowmapping uses frustum culling there can be cases where the light is behind the player it might not show some shadows that should be there.
		//for (int iObj = 0; iObj < gIndexArray.length(); iObj++)
		//{
		//	lightManager.updateMatrixBuffer(objects[gIndexArray[iObj]].getWorldMatrix(), iLight);
		//	objects[gIndexArray[iObj]].draw();
		//}

		//terrain
		lightManager.updateMatrixBuffer(terrain.getWorldMatrix(), iLight);
		terrain.draw();
	}
} 

bool updateChangedObjects()
{
	bool somethingHasChanged = false;
	// See if any objecs has moved and needs to update
	for (int i = 0; i < objects.length(); i++)
	{
		if (objects[i].popIfChanged())
		{
			gQuadTree.updateObj(objects[i].getBoundingBoxPos(), objects[i].getRotatedBoundingBoxSize(), i);
			somethingHasChanged = true;
		}
	}
	return somethingHasChanged;
}

void checkFrustumQuadTreeIntersection()
{

	gIndexArray.reset();
	Frustum frustum;
	frustum.constructFrustum(cameraPosition, cameraForward, viewData.up, viewData.fowAngle, viewData.aspectRatio, viewData.nearZ, viewData.farZ);
	
	gQuadTree.checkAgainstFrustum(gIndexArray, frustum);
}

void updateFrustumPoints(float3 camPos, float3 camDir, float3 up, float fowAngle, float aspectRatio, float nearZ, float farZ)
{
	camDir.Normalize();
	float3 middleFar = camPos + camDir * farZ;
	float3 middleNear = camPos + camDir * nearZ;

	float3 vectorLeft = camDir.Cross(up);
	vectorLeft.Normalize();
	float3 vectorDown = camDir.Cross(vectorLeft);
	vectorDown.Normalize();

	float halfHeightFar = farZ * tan(fowAngle / 2);
	float halfWidthFar = halfHeightFar * aspectRatio;

	float halfHeightNear = nearZ * tan(fowAngle / 2);
	float halfWidthNear = halfHeightNear * aspectRatio;

	float3 pointLeftUpFar = middleFar + vectorLeft * halfWidthFar - vectorDown * halfHeightFar;
	float3 pointRightUpFar = middleFar - vectorLeft * halfWidthFar - vectorDown * halfHeightFar;
	float3 pointLeftBottomFar = middleFar + vectorLeft * halfWidthFar + vectorDown * halfHeightFar;
	float3 pointRightBottomFar = middleFar - vectorLeft * halfWidthFar + vectorDown * halfHeightFar;

	float3 pointLeftUpNear =		middleNear + vectorLeft * halfWidthNear - vectorDown * halfHeightNear;
	float3 pointRightUpNear =		middleNear - vectorLeft * halfWidthNear - vectorDown * halfHeightNear;
	float3 pointLeftBottomNear =	middleNear + vectorLeft * halfWidthNear + vectorDown * halfHeightNear;
	float3 pointRightBottomNear =	middleNear - vectorLeft * halfWidthNear + vectorDown * halfHeightNear;
	


	objects[0].setRotation(float3(cameraRotation.x,cameraRotation.y,0));
	objects[0].setPosition(camPos);
	objects[1].setPosition(pointLeftUpFar);
	objects[2].setPosition(pointRightUpFar);
	objects[3].setPosition(pointLeftBottomFar);
	objects[4].setPosition(pointRightBottomFar);

	objects[5].setPosition(pointLeftUpNear);
	objects[6].setPosition(pointRightUpNear);
	objects[7].setPosition(pointLeftBottomNear);
	objects[8].setPosition(pointRightBottomNear);
}

void Render() {
	gDeviceContext->VSSetConstantBuffers(0, 1, &gMatrixBuffer);
	lightManager.updateLightBuffer();
	lightManager.bindLightBuffer();
	// clear the back buffer to a deep blue
	float3 darkBlue = float3(25.0f / 255, 25.0f / 255, 60.0f / 255)*0.5;
	float clearColor[] = { darkBlue.x,darkBlue.y,darkBlue.z, 1 };
	//bind and clear renderTargets(color,normal,position,specular maps)
	gDeferred.BindFirstPass(gDeviceContext,gDepthStencilView);
	
	// Front to back culling. Only consider those in frustum
	// Can be tested by not rendering the first or last few objects
	FrontToBack frontToBack;
	float distance = 0;
	for (int i = 0; i < gIndexArray.length(); i++)
	{
		distance = ( cameraPosition - objects[gIndexArray[i]].getPosition() ).Length();
		frontToBack.insert(distance, gIndexArray[i]);
	}
	Array<int> sortedIndexArray = frontToBack.getSortedIndexArray();
	
				//DRAW
	//objects
	shader_object.bindShadersAndLayout();	
	for (int i = 50 * gShowFrontToBack; i < sortedIndexArray.length(); i++)
	{
		updateMatrixBuffer(objects[sortedIndexArray[i]].getWorldMatrix());
		objects[sortedIndexArray[i]].draw();
	}

	//lights
	shader_object_onlyMesh.bindShadersAndLayout();
	for (int i = 0; i < lightManager.lightCount(); i++)
	{
		sphere.setPosition(lightManager.getLightPosition(i));
		sphere.setScale(float3(1, 1, 1));
		updateMatrixBuffer(sphere.getWorldMatrix());
		sphere.draw();
	}
	//mousePicking sphere
	shader_object_onlyMesh.bindShadersAndLayout();
	sphere.setScale(float3(1, 1, 1)*0.1);
	sphere.setPosition(lookPos);
	updateMatrixBuffer(sphere.getWorldMatrix());
	sphere.draw();
	//terrain
	lightManager.bindShaderResourceDepthViews();
	shader_terrain.bindShadersAndLayout();
	updateMatrixBuffer(terrain.getWorldMatrix());
	terrain.draw();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	mouse->SetWindow(wndHandle);
	mouse->SetVisible(false);

	//srand(time(NULL));
	m_keyboard = std::make_unique<Keyboard>();

	CoInitialize(nullptr);

	if (wndHandle)
	{

		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context
		gDeferred.initDeferred(gDevice);
		SetViewport(Win_WIDTH, Win_HEIGHT); //3. Sätt viewport

		CreateCameraBuffer();


		CreateMatrixDataBuffer();
		
		bool creationCheck = textureBlurrer.initilize(DXGI_FORMAT_R8G8B8A8_UNORM, L"GaussianHorizontalBlur.hlsl", L"GaussianVerticalBlur.hlsl");
		//creationCheck = glowBlurrer.initilize(DXGI_FORMAT_R32G32B32A32_FLOAT, L"GaussianHorizontalBlur.hlsl", L"GaussianVerticalBlur.hlsl");

		lightManager.createShaderForShadowMap(L"Effects/Vertex_Light.hlsl", nullptr, nullptr);
		lightManager.addLight(float3(7, 10, 7), float3(1, 1, 1), 1, float3(0, 0, 0),XM_PI*0.45,0.01,50);
		lightManager.addLight(float3(-7, 10, 7), float3(1, 0.9, 0.7), 1, float3(0, 0, 0), XM_PI*0.45, 0.01, 50);
		lightManager.createBuffers();

		terrain.create(XMINT2(500, 500), 15, 5, L"Images/heightMap2.png", smoothShading);

		//meshes
		meshes.appendCapacity(100);//CANNOT COPY MESH OBJECT
		meshes.add(Mesh()); meshes[0].loadMesh("Meshes/Sword", smoothShading);
		meshes.add(Mesh()); meshes[1].loadMesh("Meshes/sphere", smoothShading);
		meshes.add(Mesh()); meshes[2].loadMesh("Meshes/cube", flatShading);
		meshes.add(Mesh()); meshes[3].loadMesh("Meshes/tree1", smoothShading);
		meshes.add(Mesh()); meshes[4].loadMesh("Meshes/rock1", smoothShading);
		meshes.add(Mesh()); meshes[5].loadMesh("Meshes/pineTree", smoothShading);
		meshes.add(Mesh()); meshes[6].loadMesh("Meshes/cottage", flatShading);

		float3 s = terrain.getTerrainSize();
		float3 scale(0.05,0.05,0.05);
		scale *= 2;
		objects.appendCapacity(1000);

		// Frustum cubes to more easily see the frustum
		for (int i = 0; i < 9; i++)
		{
			Object frustumCube;
			frustumCube.setScale(float3(0.2,0.2,0.2));
			if(i == 0) frustumCube.setScale(float3(0.2, 0.2, 0.2)*0);
			else if(i>4) frustumCube.setScale(float3(0.2, 0.2, 0.2)*0.1);
			
			frustumCube.giveMesh(&meshes[2]);
			objects.add(frustumCube);
		}

		
		int nrOfItemsToAdd = 100;

		for (int i = 0; i < 0; i++)
		{
			Object swd = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().z / 2, terrain.getTerrainSize().z / 2)),
				float3(0, random(0, 3.14 * 2), 0),
				float3(0.1, 0.1, 0.1),
				&meshes[0]);
			objects.add(swd);
		}
		for (int i = 0; i < nrOfItemsToAdd; i++)
		{
			Object tree = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().z / 2, terrain.getTerrainSize().z / 2)),
				float3(0, random(0, 3.14 * 2), 0),
				scale,
				&meshes[3]
			);
			objects.add(tree);
		}
		for (int i = 0; i < nrOfItemsToAdd; i++)
		{
			Object rock = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().z / 2, terrain.getTerrainSize().z / 2)),
				float3(0, random(0, 3.14 * 2), 0),
				scale,
				&meshes[4]
			);
			objects.add(rock);
		}
		for (int i = 0; i < nrOfItemsToAdd; i++)
		{
			Object pineTree = Object(
				terrain.getPointOnTerrainFromCoordinates(random(-terrain.getTerrainSize().x / 2, terrain.getTerrainSize().x / 2), random(-terrain.getTerrainSize().z / 2, terrain.getTerrainSize().z / 2)),
				float3(0, random(0, 3.14 * 2), 0),
				scale,
				&meshes[5]
			);
			objects.add(pineTree);
		}
		//House
		objects.add(Object(terrain.getPointOnTerrainFromCoordinates(5,5), float3(0, 3.14, 0), scale*2, &meshes[6]));

		sphere.giveMesh(&meshes[1]);
		cube.giveMesh(&meshes[2]);

		shader_object.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment.hlsl");
		shader_object_onlyMesh.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment_onlyMesh.hlsl");
		shader_terrain.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment_Terrain.hlsl");
		gShader_Deferred.createShaders(L"Effects/Vertex_Deferred.hlsl", nullptr, L"Effects/Fragment_Deferred.hlsl");
		gDeferred.setShaderSet(gShader_Deferred);
		ShowWindow(wndHandle, nCmdShow);

		// Inserts objects in quadtree and partitions it.
		for (int i = 0; i < objects.length(); i++)
		{
			gQuadTree.insertToRoot(objects[i].getBoundingBoxPos(), objects[i].getRotatedBoundingBoxSize(), i);
		}

		// Initial rendering of shadowmap. 
		SetViewport(SMAP_WIDTH, SMAP_HEIGHT);
		drawToShadowMap();
		// Set viewPort back to normal
		SetViewport(Win_WIDTH, Win_HEIGHT);

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
				mousePos = float2(newPos.x, newPos.y);//save cursor position

				cameraRotation += float2(diff.y, diff.x)*0.002;//add mouse rotation

				if (mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED)mousePicking((float)Win_WIDTH / 2, (float)Win_HEIGHT / 2);
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
				cameraForward = XMVector3Transform(float4(0, 0, 1, 1), rotMat);
				//camera movement
				float speed = 1;
				float jumpForce = 2;
				float3 left = cameraForward.Cross(float3(0, 1, 0));
				float3 forward = XMVector3Transform(float4(0, 0, 1, 1), float4x4::CreateRotationY(cameraRotation.y));
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
				if (!grounded)velocity += gravityDirection * gravityForce * deltaTime;//dont apply gravity if on ground
				cameraPosition += movement * deltaTime + velocity * deltaTime;
				//collision, only affects y-axis
				float3 nextPos = cameraPosition + float3(0, -1, 0);
				float hy = terrain.getHeightOfTerrainFromCoordinates(nextPos.x, nextPos.z);
				if (nextPos.y < hy) {//if below terrain then add force up
					cameraPosition.y += (hy - nextPos.y) * deltaTime * 5;
					grounded = true;
					velocity = float3(0, 0, 0);
				}
				else grounded = false;

				// Toggle showcase effects
				// Changes view mode between 1:st and 3:rd person
				if (kb.V) 
				{
					if (gFirstPersonPressed == false) 
					{
						gFirstPerson = 1 - gFirstPerson;
						gFirstPersonPressed = true;
					}
				}
				else 
					gFirstPersonPressed = false;
				// Adds view frustum corners with closer far plane
				if (kb.F)
				{
					if (gShowFrustumPressed == false)
					{
						gShowFrustum = 1 - gShowFrustum;
						gShowFrustumPressed = true;
						for (int i = 0; i < 9; i++)
						{
							objects[i].setPosition(float3(0, 0, 0));
						}
					}
				}
				else
					gShowFrustumPressed = false;

				// Shows front to back by not rendering the closer objects
				if (kb.B)
				{
					if (gShowFrontToBackPressed == false)
					{
						gShowFrontToBack = 1 - gShowFrontToBack;
						gShowFrontToBackPressed = true;
					}
				}
				else
					gShowFrontToBackPressed = false;

				//frustum balls
				if(gShowFrustum)
					updateFrustumPoints(cameraPosition,cameraForward, viewData.up,viewData.fowAngle,viewData.aspectRatio, viewData.nearZ, 3);
				//rotate
				rotation += deltaTime * XM_2PI*0.25*(1.0f / 4);
				//update cameradata buffer
				XMFLOAT4 cpD = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1);
				gDeviceContext->UpdateSubresource(gCameraBuffer, 0, 0, &cpD, 0, 0);

				bool someObjectHasChanged = updateChangedObjects();
				//Frustum cull
				checkFrustumQuadTreeIntersection();
				//draw shadow maps
				if (someObjectHasChanged)
				{
					SetViewport(SMAP_WIDTH, SMAP_HEIGHT);
					drawToShadowMap();
					// Return viewport back to normal
					SetViewport(Win_WIDTH, Win_HEIGHT);
				}
				//draw deferred maps
				Render();
				//draw deferred maps to full quad
				gDeferred.BindSecondPass(gDeviceContext, gBackbufferRTV, gCameraBuffer);

				//blur backBuffer
				if (state.rightButton) {
					ID3D11Resource* r;
					gBackbufferRTV->GetResource(&r);
					textureBlurrer.blurTexture(r,Win_WIDTH,Win_HEIGHT);
				}

				gSwapChain->Present(0, 0); //9. Växla front- och back-buffer
			}
			time = clock() - time;
			deltaTime = (float)time / 1000.0f;
		}

		CoUninitialize();

		gMatrixBuffer->Release();

		gDepthStencilView->Release();
		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();

		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		ZeroMemory(&viewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
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