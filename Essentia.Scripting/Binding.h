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

	class Binding
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

		WrenForeignMethodFn GetMethod(const char* fullSignature)
		{
			if (methods.find(fullSignature) != methods.end())
				return methods[fullSignature];
			else 
				return nullptr;
		}

		static Binding& GetInstance()
		{
			static Binding instance;
			return instance;
		}

	protected:

		static Binding* instance;
		std::unordered_map<std::string, WrenForeignMethodFn> methods;
	};

	WrenForeignMethodFn BindForeignMethod(WrenVM* vm,
		const char* module,
		const char* className,
		bool isStatic,
		const char* signature
	)
	{
		std::string fullSignature{ module };
		fullSignature += className;
		fullSignature += signature;
		if (isStatic) {
			fullSignature += "s";
		}

		return Binding::GetInstance().GetMethod(fullSignature.c_str());
	}
}
