#include "Binding.h"


WrenForeignMethodFn es::BindForeignMethod(WrenVM* vm,
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

	return es::Binding::GetInstance().GetMethod(fullSignature.c_str());
}

WrenForeignClassMethods es::BindForeignClass(WrenVM*, const char* module, const char* className)
{
	std::string identifier{ module };
	identifier += className;
	return es::Binding::GetInstance().GetForeignClassMethods(identifier.c_str());
}