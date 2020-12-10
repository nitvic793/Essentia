#pragma once

#include "Component.h"
#include "Interface.h"

enum class FieldTypes
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
			T origComp = *comp;
			Visit(comp, visitor);
			return HasComponentChanged(comp, &origComp);
		};

		componentFactoryMap[T::ComponentName] = [&](ComponentManager* componentManager)->ComponentPoolBase*
		{
			return componentManager->GetOrCreatePool<T>();
		};
	}

	template<typename T>
	void RegisterComponent(std::function<void(T*, IVisitor*)>&& vistorFunc)
	{
		componentVisitorMap[T::ComponentName] = [&](IComponent* component, IVisitor* visitor)
		{
			T* comp = (T*)component;
			T origComp = *comp;
			vistorFunc(comp, visitor);
			return HasComponentChanged(comp, &origComp);
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
			T origComp = *comp;
			VisitFields(comp, visitor, fields);
			return HasComponentChanged(comp, &origComp);
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

	bool VisitFields(const char* componentName, IComponent* component, IVisitor* visitor)
	{
		if (componentVisitorMap.find(componentName) != componentVisitorMap.end())
		{
			return componentVisitorMap[componentName](component, visitor);
		}

		return false;
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
	std::unordered_map<std::string, std::function<bool(IComponent*, IVisitor*)>> componentVisitorMap;
	std::unordered_map<std::string, std::function<ComponentPoolBase* (ComponentManager*)>> componentFactoryMap;

	template<typename T>
	void VisitFields(T* component, IVisitor* visitor, std::vector<Field> fields)
	{
		for (const Field& field : fields)
		{
			switch (field.FieldType)
			{
			case FieldTypes::kFieldTypeFloat:
				visitor->Visit(T::ComponentName, field.FieldName.c_str(), *(float*)((char*)component + field.Offset));
				break;
			case FieldTypes::kFieldTypeInt32:
				visitor->Visit(T::ComponentName, field.FieldName.c_str(), *(int32*)((char*)component + field.Offset));
				break;
			case FieldTypes::kFieldTypeFloat3:
				visitor->Visit(T::ComponentName, field.FieldName.c_str(), *(DirectX::XMFLOAT3*)((char*)component + field.Offset));
				break;
			default:
				break;
			}
		}
	}

	template<typename T>
	bool HasComponentChanged(T* comp1, T* comp2)
	{
		int result = memcmp(comp1, comp2, sizeof(T));
		if (result != 0 && strcmp(T::ComponentName, "SelectedComponent") != 0)
		{
			return true;
		}

		return false;
	}
};

extern ComponentReflector GComponentReflector;