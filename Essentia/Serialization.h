#pragma once

#include "Interface.h"

void Save(const TransformRef& transform, const char* fname);

void SaveResources(ResourcePack& resources, const char* fname);

void SaveEntities(std::vector<EntityInterface>& entities, const char* fname);

void RegisterComponents();