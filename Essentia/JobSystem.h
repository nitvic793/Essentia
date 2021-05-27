#pragma once

#include "Declarations.h"

namespace es::async
{
	using Job = void (*)(void*, uint32);

	struct JobHandle
	{
		uint32 JobID;
	};

	void InitJobSystem();

	JobHandle ExecuteJob(Job&& job);

	JobHandle ExecuteJob(Job&& job, JobHandle dependency);

}
