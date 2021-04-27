#pragma once

#include <string>
#include <unordered_map>
#include <wren.hpp>

template<class...>struct types { using type = types; };

namespace es
{
	WrenForeignMethodFn BindForeignMethod(WrenVM* vm,
		const char* module,
		const char* className,
		bool isStatic,
		const char* signature
	);

	WrenForeignClassMethods BindForeignClass(WrenVM*,
		const char* module,
		const char* className
	);

	class ScriptBinding
	{
	public:
		void BindMethod(const char* module, const char* className, const char* signature, bool isStatic, WrenForeignMethodFn fn)
		{
			std::string fullSignature{ module };
			fullSignature += className;
			fullSignature += signature;
			if (isStatic) {
				fullSignature += "s";
			}

			methods[fullSignature] = fn;
		}

		void BindForeignClass(const char* module, const char* className, WrenForeignClassMethods classMethods)
		{
			std::string identifier{ module };
			identifier += className;
			classes[identifier] = classMethods;
		}

		WrenForeignClassMethods GetForeignClassMethods(const char* identifier)
		{
			if (classes.find(identifier) != classes.end())
				return classes[identifier];
			else
				return WrenForeignClassMethods{ nullptr, nullptr };
		}

		WrenForeignMethodFn GetMethod(const char* fullSignature)
		{
			if (methods.find(fullSignature) != methods.end())
				return methods[fullSignature];
			else 
				return nullptr;
		}

		static ScriptBinding& GetInstance()
		{
			static ScriptBinding instance;
			return instance;
		}

	protected:

		static ScriptBinding* instance;
		std::unordered_map<std::string, WrenForeignMethodFn> methods;
		std::unordered_map<std::string, WrenForeignClassMethods> classes;
	};
}
