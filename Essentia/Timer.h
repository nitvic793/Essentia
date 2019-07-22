#pragma once

#include "Declarations.h"
#include <algorithm>

class Timer
{
public:
	void Start()
	{
		__int64 perfFreq;
		QueryPerformanceFrequency((LARGE_INTEGER*)& perfFreq);
		perfCounterSeconds = 1.0 / (double)perfFreq;

		__int64 now;
		QueryPerformanceCounter((LARGE_INTEGER*)& now);
		startTime = now;
		currentTime = now;
		previousTime = now;
		frameCounter = 0;

		DeltaTime = 0.f;
		TotalTime = 0.f;
	}

	void Tick()
	{
		frameCounter++;
		__int64 now;
		QueryPerformanceCounter((LARGE_INTEGER*)& now);
		currentTime = now;

		DeltaTime = std::max(((currentTime - previousTime) * perfCounterSeconds), 0.0);
		TotalTime = ((currentTime - startTime) * perfCounterSeconds);
		previousTime = currentTime;
	}

	double DeltaTime;
	double TotalTime;

private:
	uint64 startTime;
	uint64 currentTime;
	uint64 previousTime;
	uint64 frameCounter;

	double perfCounterSeconds;
};

