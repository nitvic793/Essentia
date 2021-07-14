#pragma once

#include "Declarations.h"
#include <functional>

namespace es::jobs
{
    static constexpr uint32 CDefaultThreadCount = 3;
    static constexpr uint64 CMaxJobs = std::numeric_limits<uint64>::max() - 1;
    using JobFunction = std::function<void(void*)>;

    struct JobHandle
    {
        static constexpr uint64 kNoneJobHandle = 0U;
        uint64 JobID;
    };

    static constexpr JobHandle CNoneJob = { JobHandle::kNoneJobHandle };

    struct Job
    {
        JobFunction     mJobFunction;
        void*           mArgs;
        JobHandle       mDependency = CNoneJob;
        bool            mIsFinished = false;

        static Job Create(JobFunction job, void* args = nullptr)
        {
            return { job, args };
        }

        static Job Create(JobFunction job, void* args, JobHandle dependency)
        {
            return { job, args, dependency };
        }
    };

    // Note: Defaults to hardware thread count if threadCount = 0
    void InitJobSystem(uint32 threadCount = CDefaultThreadCount, bool useHardwareThreadCount = false); 

    void DestroyJobSystem();

    JobHandle Execute(Job&& job);
    JobHandle Execute(JobFunction&& jobFunc, void* args = nullptr, JobHandle dependency = CNoneJob);
    bool      IsFinished(JobHandle job);

    void WaitForAll();

    void WaitForJob(JobHandle handle);
}
