#pragma once
#include "standardClasses.h"

#define maxLightCount 10
#define SMAP_WIDTH  (Win_WIDTH  * 3)
#define SMAP_HEIGHT (Win_HEIGHT * 3)

struct ShaderLight {
	float4 position = float4(0, 0, 0, 0);
	float4 color = float4(0, 0, 0, 0);//.a is intensity
	float4x4 viewPerspectiveMatrix;
};
struct ShaderLights {
	float4 lightCount = float4(0, 0, 0, 0);
	ShaderLight lights[maxLightCount];
	float4 smapSize = float4(SMAP_WIDTH, SMAP_HEIGHT, 0, 0);
};

class LightManager
{
private:
	ShaderLights shadowMapLights;
	Array<float4x4> matrixViews;
	Array<float4x4> matrixPerspective;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11DepthStencilView* shadowMaps[maxLightCount];
	ID3D11ShaderResourceView* shaderResourceViewsDepth[maxLightCount];

	ShaderSet shader_shadowMap;

	void releaseBuffers();
public:
	void updateLightBuffer();
	void updateMatrixBuffer(float4x4 worldMatrix, int index);
	void bindShaderResourceDepthViews();
	void bindLightBuffer();
	void bindMatrixBuffer();
	float4x4 getLightViewMatrix(int index) const;
	void setShadowMapForDrawing(int index);
	void createShaderForShadowMap(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0);
	void createBuffers();
	int lightCount()const;
	void addLight(float3 position, float3 color, float intensity, float3 lookAt = float3(0, 0, 0), float FOV = XM_PI*0.45, float nearPlane = 0.01, float farPlane = 50);
	float3 getLightPosition(int index);
	LightManager();
	~LightManager();
};