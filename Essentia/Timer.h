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

		DeltaTime = std::max((float)((currentTime - previousTime) * perfCounterSeconds), 0.f);
		TotalTime = (float)((currentTime - startTime) * perfCounterSeconds);
		previousTime = currentTime;
	}

	static const Timer *const GetInstance() 
	{
		return Instance;
	}

	float DeltaTime;
	float TotalTime;

private:
	Timer() 
	{
		Instance = this;
	};

	uint64 startTime = 0;
	uint64 currentTime = 0;
	uint64 previousTime = 0;
	uint64 frameCounter = 0;

	double perfCounterSeconds = 0;

	static Timer* Instance;
	friend class Game;
};

