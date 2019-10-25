#pragma once
#include "Memory.h"
#include "EntityBase.h"
#include <functional>
#include <unordered_map>

using ComponentGenFunction = std::function<IComponent* (std::string_view)>;

class ComponentFactory
{
public:
private:
	std::unordered_map<std::string_view, ComponentGenFunction> componentGenerators;
};

extern ComponentFactory GComponentFactory;