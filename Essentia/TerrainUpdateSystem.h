#pragma once

#include "System.h"

class TerrainUpdateSystem : public ISystem
{
public:
	virtual void Initialize() override;
	virtual void Update(float dt, float totalTime) override;
private:
};