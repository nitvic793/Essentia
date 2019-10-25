#pragma once
#include "Memory.h"
#include "Component.h"
#include "EntityBase.h"
#include <functional>
#include <unordered_map>

using ComponentGenFunction = std::function<IComponent* ()>;
using ComponentPoolGenFunction = std::function<ComponentPoolBase* ()>;

template<typename T>
void RegisterComponentType();

class ComponentFactory
{
public:
	void				RegisterComponentPool(const char* componentName, ComponentPoolGenFunction generator);
	void				RegisterComponent(const char* componentName, ComponentGenFunction generator);
	IComponent*			CreateComponent(const char* componentName);
	ComponentPoolBase*	CreateComponentPool(const char* componentName);
private:
	std::unordered_map<std::string_view, ComponentGenFunction> componentGenerators;
	std::unordered_map<std::string_view, ComponentPoolGenFunction> componentPoolGenerators;
};

extern ComponentFactory GComponentFactory;

template<class T>
void RegisterComponentType(const char* name)
{
	GComponentFactory.RegisterComponent(name, [&]()->IComponent * 
		{
			return (IComponent*)Mem::Alloc<T>();
		});

	GComponentFactory.RegisterComponentPool(name, [&]()->ComponentPoolBase*
		{
			return (ComponentPoolBase*)Mem::Alloc<ComponentPool<T>>();
		});
}
