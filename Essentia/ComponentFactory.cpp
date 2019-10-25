#include "pch.h"
#include "ComponentFactory.h"

ComponentFactory GComponentFactory;

void ComponentFactory::RegisterComponentPool(const char* componentName, ComponentPoolGenFunction generator)
{
	componentPoolGenerators[componentName] = generator;
}

void ComponentFactory::RegisterComponent(const char* componentName, ComponentGenFunction generator)
{
	componentGenerators[componentName] = generator;
}

IComponent* ComponentFactory::CreateComponent(const char* componentName)
{
	auto generator = componentGenerators[componentName];
	return generator();
}

ComponentPoolBase* ComponentFactory::CreateComponentPool(const char* componentName)
{
	auto generator = componentPoolGenerators[componentName];
	return generator();
}
