#include "ConstantBufferViewPool.h"

void ConstantBufferViewPool::Reset()
{
	for (auto& pool : cbvPools)
	{
		pool.second.CurrentIndex = 0;
	}
}
