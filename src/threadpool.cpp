#include "threadpool.h"
#include "log.h"

using namespace pml;

ThreadPool& ThreadPool::Get()
{
    static ThreadPool pool;
    return pool;
}

ThreadPool::ThreadPool() :
    m_bDone(false)
{
    AddWorkers(1);    //we want at least 1 threaad otherwise why are we calling this
}


size_t ThreadPool::CreateWorkers(size_t nMinThreads, size_t nMaxThreads)
{
    auto nThreads = std::max(nMinThreads, std::min(nMaxThreads, std::thread::hardware_concurrency()));
    if(nThreads > m_vThreads.size())
    {
        return AddWorkers(nThreads-m_vThreads.size());
    }
    return m_vThreads.size();
}

size_t ThreadPool::AddWorkers(size_t nWorkers)
{
   try
    {
        for(size_t i = 0; i < nWorkers; i++)
        {
            m_vThreads.push_back(std::thread(&ThreadPool::WorkerThread, this));
        }
        return m_vThreads.size();
    }
    catch(std::exception& e)
    {
        return m_vThreads.size();
    }
}

ThreadPool::~ThreadPool()
{
    m_bDone = true;
    m_condition.notify_all();
    for(auto& th : m_vThreads)
    {
        th.join();
    }
}

void ThreadPool::WorkerThread()
{
    while(!m_bDone)
    {
        std::function<void()> task(nullptr);

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]{return m_bDone || !m_qWork.empty();});
            if(!m_bDone && m_qWork.empty() == false)
            {
                task = std::move(m_qWork.front());
                m_qWork.pop();
            }
        }
        if(task)
        {
            task();
        }
    }
}



