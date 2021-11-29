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
    auto nThreads = std::thread::hardware_concurrency();
    pmlLog() << "pml ThreadPool: Creating " << nThreads << " threads";
    try
    {
        for(size_t i = 0; i < nThreads; i++)
        {
            m_vThreads.push_back(std::thread(&ThreadPool::WorkerThread, this));
        }
    }
    catch(std::exception& e)
    {
        m_bDone = true;
        throw;
    }
}

ThreadPool::~ThreadPool()
{
    m_bDone = true;
    for(auto& th : m_vThreads)
    {
        th.join();
    }
}

void ThreadPool::WorkerThread()
{
    while(!m_bDone)
    {
        std::shared_ptr<std::function<void()>> task = m_qWork.try_pop();
        if(task)
        {
            task.get();
        }
        else
        {
            std::this_thread::yield();
        }
    }
}
