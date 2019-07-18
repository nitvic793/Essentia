#include "pch.h"
#include <locale>
#include <codecvt>
#include <sstream>

std::wstring ToWString(const std::string& str)
{
	std::wstringstream cls;
	cls << str.c_str();
	std::wstring total = cls.str();
	return total;
}
