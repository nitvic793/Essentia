#pragma once

#include "Component.h"
#include "Interface.h"


enum FieldTypes
{
	kFieldTypeInt32,
	kFieldTypeInt64,
	kFieldTypeFloat,
	kFieldTypeFloat3,
	kFieldTypeFloat4,
	kFieldTypeComplex
};

struct Field
{
	FieldTypes			FieldType;
	uint32				Offset;
	std::string			FieldName;
	std::string			TypeName;
};

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

	template<typename T>
	void RegisterComponent(std::function<void(T*, IVisitor*)> &&vistorFunc)
	{
		componentVisitorMap[T::ComponentName] = [&](IComponent* component, IVisitor* visitor)
		{
			T* comp = (T*)component;
			vistorFunc(comp, visitor);
		};

		componentFactoryMap[T::ComponentName] = [](ComponentManager* componentManager)->ComponentPoolBase*
		{
			return componentManager->GetOrCreatePool<T>();
		};
	}

	template<typename T>
	void RegisterComponent(std::vector<Field> fields)
	{
		componentVisitorMap[T::ComponentName] = [=](IComponent* component, IVisitor* visitor)
		{
			T* comp = (T*)component;
			VisitFields(comp, visitor, fields);
		};

		componentFactoryMap[T::ComponentName] = [](ComponentManager* componentManager)->ComponentPoolBase*
		{
			return componentManager->GetOrCreatePool<T>();
		};
	}

	ComponentPoolBase* CreatePool(std::string_view componentName, ComponentManager* componentManager)
	{
		return componentFactoryMap[std::string(componentName)](componentManager);
	}

	void VisitFields(const char* componentName, IComponent* component, IVisitor* visitor)
	{
		if (componentVisitorMap.find(componentName) != componentVisitorMap.end())
			componentVisitorMap[componentName](component, visitor);
	}

	void CleanUp()
	{
		componentVisitorMap.clear();
		componentFactoryMap.clear();
	}

	~ComponentReflector()
	{
 	}
private:
	std::unordered_map<std::string, std::function<void(IComponent*, IVisitor*)>> componentVisitorMap;
	std::unordered_map<std::string, std::function<ComponentPoolBase * (ComponentManager*)>> componentFactoryMap;

	template<typename T>
	void VisitFields(T* component, IVisitor* visitor, std::vector<Field> fields)
	{
		for (const Field& field : fields)
		{
			switch (field.FieldType)
			{
				case kFieldTypeFloat:
					visitor->Visit(T::ComponentName, field.FieldName.c_str(), *(float*)(component + field.Offset));
					break;
				case kFieldTypeInt32:
					visitor->Visit(T::ComponentName, field.FieldName.c_str(), *(int32*)((char*)component + field.Offset));
					break;
				default:
					break;
			}
		}
	}
};

extern ComponentReflector GComponentReflector;