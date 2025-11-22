#include "Prereq.h"
#include "Jobs.h"

namespace Voidstar
{
    JobSystem::JobSystem()
        : m_running(false)
    {
    }

    JobSystem::~JobSystem()
    {
        Stop();
    }

    void JobSystem::Start(unsigned int threadCount)
    {
        if (m_running)
            return;

        m_running = true;
        if (threadCount <= 0)
            threadCount = 1;

        m_workers.reserve(threadCount);
        for (unsigned int i = 0; i < threadCount; ++i)
        {
            m_workers.emplace_back(&JobSystem::WorkerLoop, this);
        }
    }

    void JobSystem::Stop()
    {
        if (!m_running)
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }

        m_cv.notify_all();

        for (auto& t : m_workers)
        {
            if (t.joinable())
                t.join();
        }
        m_workers.clear();
    }

    void JobSystem::PushJob(const std::function<void()>& job)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_jobs.push(job);
        }
        m_cv.notify_one();
    }

    void JobSystem::WorkerLoop()
    {
        while (true)
        {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [&] {
                    return !m_jobs.empty() || !m_running;
                    });

                if (!m_running && m_jobs.empty())
                    return;

                job = std::move(m_jobs.front());
                m_jobs.pop();
            }

            job(); 
        }
    }

}