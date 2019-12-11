#pragma once

#include "Interface.h"

void SaveResources(ResourcePack& resources, const char* fname);

void SaveEntities(std::vector<EntityInterface>& entities, const char* fname);

void SaveScene(Scene&& scene, const char* fname);

void RegisterComponents();

/*
Serialization for predefined types
*/
