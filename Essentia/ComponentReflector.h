#pragma once

#include "Component.h"
#include "Interface.h"

class ComponentReflector
{
public:
	template<typename T>
	void RegisterComponent()
	{
		componentVisitorMap[T::ComponentName] = [&](IComponent* component, IVisitor* visitor)
		{
			T* comp = (T*)component;
			Visit(comp, visitor);
		};

		componentFactoryMap[T::ComponentName] = [&](ComponentManager* componentManager)->ComponentPoolBase*
		{
			return componentManager->GetOrCreatePool<T>();
		};
	}

	ComponentPoolBase* CreatePool(std::string_view componentName, ComponentManager* componentManager)
	{
		return componentFactoryMap[componentName](componentManager);
	}

	void VisitFields(IComponent* component, IVisitor* visitor)
	{
		auto name = component->GetName();
		if (componentVisitorMap.find(name) != componentVisitorMap.end())
			componentVisitorMap[name](component, visitor);
	}
private:
	std::unordered_map<std::string_view, std::function<void(IComponent*, IVisitor*)>> componentVisitorMap;
	std::unordered_map<std::string_view, std::function<ComponentPoolBase * (ComponentManager*)>> componentFactoryMap;
};

extern ComponentReflector GComponentReflector;