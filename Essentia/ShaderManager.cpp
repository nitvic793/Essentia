#include "pch.h"
#include "ShaderManager.h"

D3D12_SHADER_BYTECODE ShaderManager::LoadShader(std::wstring shaderCsoFile)
{
	ID3DBlob* shaderBlob;
	D3DReadFileToBlob(shaderCsoFile.c_str(), &shaderBlob);
	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.BytecodeLength = shaderBlob->GetBufferSize();
	shaderBytecode.pShaderBytecode = shaderBlob->GetBufferPointer();
	//Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
	//D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_PPV_ARGS(reflection.GetAddressOf()));
	//D3D12_SHADER_BUFFER_DESC desc;
	//D3D12_SHADER_DESC shaderDesc;
	//D3D12_SHADER_VARIABLE_DESC varDesc;
	//D3D12_SHADER_TYPE_DESC typeDesc;
	//reflection->GetConstantBufferByIndex(0)->GetDesc(&desc);
	//reflection->GetConstantBufferByIndex(0)->GetVariableByIndex(0)->GetDesc(&varDesc);
	//reflection->GetConstantBufferByIndex(0)->GetVariableByIndex(0)->GetType()->GetDesc(&typeDesc);
	//reflection->GetDesc(&shaderDesc);
	return shaderBytecode;
}
