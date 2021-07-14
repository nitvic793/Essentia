
#include "JobSystem.h"
#include "ConcurrentQueue.h"
#include <unordered_map>
#include <Memory.h>
#include <EngineContext.h>

namespace es::jobs
{
    // Basic job system.
    class JobSystem
    {
    public:
        JobSystem(uint32 threadCount = CDefaultThreadCount)
            : mJobCounter(0)
            , mThreadCount(threadCount)
            , mIsRunning(false)
        {}

        void        Start();
        JobHandle   Execute(Job&& job);
        void        Stop();
        void        WaitForAll();
        void        WaitForJob(JobHandle handle);
        bool        IsJobFinished(JobHandle handle) const;

    private:
        uint64      mJobCounter;
        uint32      mThreadCount;
        bool        mIsRunning;

        ConcurrentQueue<JobHandle>          mQueue;
        std::condition_variable             mConditionVar;
        std::mutex                          mMutex;
        std::vector<std::thread>            mThreads; // All worker threads
        std::unordered_map<uint64, Job>     mJobMap;  // Keeps track of running jobs. TODO: An efficient way to store JobID -> Job map
    };

    static JobSystem* gJobSystem = nullptr;

    void InitJobSystem(uint32 threadCount, bool useHardwareThreadCount)
    {
        auto hardwareThreads = std::thread::hardware_concurrency();
        threadCount = (threadCount == 0 || useHardwareThreadCount) ? hardwareThreads : threadCount; // use hardware thread count if thread count is 0 or if flag is set
        gJobSystem = Mem::Alloc<JobSystem>(std::max(threadCount, CDefaultThreadCount));
        gJobSystem->Start();
        GContext->JobSystem = gJobSystem;
    }

    JobHandle Execute(Job&& job)
    {
        return GContext->JobSystem->Execute(std::move(job));
    }

    JobHandle Execute(JobFunction&& jobFunc, void* args, JobHandle dependency)
    {
        return Execute(std::move(Job{ jobFunc, args, dependency }));
    }

    bool IsFinished(JobHandle job)
    {
        return GContext->JobSystem->IsJobFinished(job);
    }

    void WaitForAll()
    {
        GContext->JobSystem->WaitForAll();
    }

    void WaitForJob(JobHandle handle)
    {
        GContext->JobSystem->WaitForJob(handle);
    }

    void DestroyJobSystem()
    {
        WaitForAll();
        GContext->JobSystem->Stop();
        Mem::Free(GContext->JobSystem);
        gJobSystem = nullptr;
    }

    void JobSystem::Start()
    {
        mThreads.resize(mThreadCount);
        mIsRunning = true;

        // Hacky, but does the job for now
        auto worker = [&]()
        {
            while (mIsRunning)
            {
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    if (!mIsRunning)
                        break;

                    mConditionVar.wait(lock, [&]
                        {
                            return !mIsRunning || !mQueue.IsEmpty(); // TODO: Condition may not be needed as Execute() notifies this wait() anyway?
                        });
                }

                while (!mQueue.IsEmpty())
                {
                    auto jobHandle = mQueue.Pop();
                    auto& job = mJobMap.at(jobHandle.JobID); // at() is thread safe: https://en.cppreference.com/w/cpp/container#Thread_safety
                    if (job.mDependency.JobID != JobHandle::kNoneJobHandle)
                    {
                        //std::unique_lock<std::mutex> lock(mMutex);
                        //auto jobIter = mJobMap.find(job.mDependency.JobID);
                        //lock.unlock();

                        //while (jobIter != mJobMap.end())
                        //{
                        //    std::this_thread::yield();
                        //    std::unique_lock<std::mutex> lock(mMutex);
                        //    jobIter = mJobMap.find(job.mDependency.JobID);
                        //    lock.unlock();
                        //}
                    }

                    job.mJobFunction(job.mArgs);
                    job.mIsFinished = true;

                    {
                        std::unique_lock<std::mutex> lock(mMutex);
                        mJobMap.erase(jobHandle.JobID);
                    }
                }
            }
        };

        for (uint32 i = 0; i < mThreadCount; ++i)
        {
            mThreads[i] = std::thread(worker);

#ifdef _WIN32 // Credits: https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/#comments
            // Do Windows-specific thread setup:
            HANDLE handle = (HANDLE)mThreads[i].native_handle();

            // Put each thread on to dedicated core:
            DWORD_PTR affinityMask = 1ull << i;
            DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);

            HRESULT hr = SetThreadDescription(handle, L"esJobSystemWorker");
#endif // _WIN32

            mThreads[i].detach();
        }
    }

    JobHandle JobSystem::Execute(Job&& job)
    {
        // This assumes Execute() will only by called from the main thread. Maybe atomic jobCounter would help?
        mJobCounter = (mJobCounter + 1) % CMaxJobs; // 64 bit uint, so chances of wrapping back to 0 virtually impossible.
        JobHandle handle = { mJobCounter };
        mJobMap[handle.JobID] = std::move(job);
        mQueue.Push(handle);
        mConditionVar.notify_one(); // Notify a thread that a new job is available
        return handle;
    }

    void JobSystem::Stop()
    {
        mIsRunning = false;
        mConditionVar.notify_all(); // Unblock all threads and stop
    }

    void JobSystem::WaitForAll()
    {
        while (!mJobMap.empty())
        {
            std::this_thread::yield();
        }
    }

    void JobSystem::WaitForJob(JobHandle handle)
    {
        // find() is thread safe. Ref: https://en.cppreference.com/w/cpp/container#Thread_safety
        while (mJobMap.find(handle.JobID) != mJobMap.end())
        {
            std::this_thread::yield();
        }
    }

    bool JobSystem::IsJobFinished(JobHandle handle) const
    {
        auto iter = mJobMap.find(handle.JobID);
        return iter == mJobMap.end();
    }
}

