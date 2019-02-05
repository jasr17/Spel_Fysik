#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>
#include <string>
#include "Array.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

enum typeOfShading {
	flatShading,
	smoothShading
};

struct Vertex {
	float3 position;
	float2 uv;
	float3 normal;
	Vertex(float3 _position = float3(0, 0, 0), float2 _uv = float2(0, 0), float3 _normal = float3(0, 0, 0)) {
		position = _position;
		uv = _uv;
		normal = _normal;
	}
};

float random(float min, float max) {
	return min + ((float)(rand() % 10000) / 10000)*(max - min);
}