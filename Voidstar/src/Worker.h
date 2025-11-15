#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <functional>
namespace Voidstar
{
    class Worker
    {
    public:
        Worker()
            : m_Stop(false)
        {
            m_Thread = std::thread([this] { Run(); });
        }

        ~Worker()
        {
            {
                std::lock_guard lock(m_Mutex);
                m_Stop = true;
            }
            m_Cv.notify_one();
            m_Thread.join();
        }

        template<typename F>
        void Enqueue(F&& fn)
        {
            {
                std::lock_guard lock(m_Mutex);
                m_Jobs.emplace_back(std::forward<F>(fn));
            }
            m_Cv.notify_one();
        }

    private:
        void Run()
        {
            for (;;)
            {
                std::function<void()> job;
                {
                    std::unique_lock lock(m_Mutex);
                    m_Cv.wait(lock, [&] { return m_Stop || !m_Jobs.empty(); });
                    if (m_Stop && m_Jobs.empty())
                        break;

                    job = std::move(m_Jobs.back());
                    m_Jobs.pop_back();
                }
                job();
            }
        }

        std::thread m_Thread;
        std::mutex m_Mutex;
        std::condition_variable m_Cv;
        bool m_Stop;
        std::vector<std::function<void()>> m_Jobs;
    };

}
