#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace pml
{
    class ThreadPool
    {
        public:
            static ThreadPool& Get();
            template<typename Callable, typename... Args> void Submit(Callable&& func, Args&&... args)
            {
                Submit([=]{func(args...);});
            }
            template<typename FunctionType> void Submit(FunctionType f)
            {
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_qWork.push(std::function<void()>(f));
                }
                m_condition.notify_one();

            }

            size_t CreateWorkers(size_t nMinThreads, size_t nMaxThreads);
            size_t AddWorkers(size_t nWorkers);

        private:
            ThreadPool();
            ~ThreadPool();


            void WorkerThread();

            std::atomic_bool m_bDone;
            std::queue<std::function<void()>> m_qWork;
            std::vector<std::thread> m_vThreads;

            std::mutex m_mutex;
            std::condition_variable m_condition;
    };
};
