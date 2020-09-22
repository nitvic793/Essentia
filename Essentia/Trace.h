#pragma once

#include "ImguiConsole.h"

namespace es
{
	template<typename... Args>
	static void Log(const char* fmt, Args... args);

}

template<typename ...Args>
void es::Log(const char* fmt, Args ...args)
{
	if (GConsole)
		GConsole->AddLog(fmt, args...);
}