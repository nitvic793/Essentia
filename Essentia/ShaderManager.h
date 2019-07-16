#pragma once

#include <d3d12.h>
#include <string>

class ShaderManager
{
public:
	static D3D12_SHADER_BYTECODE LoadShader(std::wstring shaderCsoFile);
};

