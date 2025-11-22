#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
namespace Voidstar
{
    class JobSystem
    {
    public:
        JobSystem();
        ~JobSystem();

        void Start(unsigned int threadCount = std::thread::hardware_concurrency() - 2);

        void Stop();

        void PushJob(const std::function<void()>& job);

    private:
        void WorkerLoop();
        std::vector<std::thread>  m_workers;
        std::queue<std::function<void()>> m_jobs;
        std::mutex  m_mutex;
        std::condition_variable m_cv;
        bool m_running;
    };

}