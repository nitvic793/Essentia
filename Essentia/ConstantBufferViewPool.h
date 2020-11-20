#pragma once

#include "EngineContext.h"
#include "ShaderResourceManager.h"
#include "Entity.h"
#include "Utility.h"

// ConstantBufferView Pool Type
struct CBVPoolType
{
	Vector<ConstantBufferView>	Pool;
	uint32						CurrentIndex = 0;
};

class ConstantBufferViewPool
{
public:
	template<typename T>
	void InitializePool(const uint32 poolSize);

	template<typename T>
	ConstantBufferView RequestConstantBufferView();

	void Reset();
private:
	std::unordered_map<std::string_view, CBVPoolType> cbvPools;
};

template<typename T>
inline void ConstantBufferViewPool::InitializePool(const uint32 poolSize)
{
	CBVPoolType cbvPool;
	cbvPool.Pool = Vector<ConstantBufferView>(poolSize);
	auto shaderResourceManager = GContext->ShaderResourceManager;
	for (uint32 i = 0; i < poolSize; ++i)
	{
		auto cbv = shaderResourceManager->CreateCBV(sizeof(T));
		cbvPool.Pool.Push(cbv);
	}

	std::string_view typeName = TypeName<T>();
	cbvPools[typeName] = std::move(cbvPool);
}

template<typename T>
inline ConstantBufferView ConstantBufferViewPool::RequestConstantBufferView()
{
	std::string_view typeName = TypeName<T>();
	CBVPoolType& pool = cbvPools[typeName];
	ConstantBufferView cbv = pool.Pool[pool.CurrentIndex];
	pool.CurrentIndex = (pool.CurrentIndex + 1) % (uint32)pool.Pool.Size();
	return cbv;
}
