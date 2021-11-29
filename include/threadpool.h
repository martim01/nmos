#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>


namespace pml
{
    template<typename T> class ThreadSafeQueue
    {

        private:
            struct node
            {
                std::shared_ptr<T> pData;
                std::unique_ptr<node> pNext;
            };
            std::mutex m_mutexHead;
            std::unique_ptr<node> m_pHead;

            std::mutex m_mutexTail;
            node* m_pTail;

            node* get_tail()
            {
                std::lock_guard<std::mutex> lg(m_mutexTail);
                return m_pTail;
            }

            std::unique_ptr<node> pop_head()
            {
                std::lock_guard<std::mutex> lg(m_mutexHead);
                if(m_pHead.get() == get_tail())
                {
                    return nullptr;
                }
                else
                {
                    std::unique_ptr<node> pOld = std::move(m_pHead);
                    m_pHead = std::move(pOld->pNext);
                    return pOld;
                }
            }

        public:
            ThreadSafeQueue() : m_pHead(new node), m_pTail(m_pHead.get())
            {}

            ThreadSafeQueue(const ThreadSafeQueue& other)=delete;
            ThreadSafeQueue& operator=(const ThreadSafeQueue& other)=delete;

            std::shared_ptr<T> try_pop()
            {
                std::unique_ptr<node> oldHead = pop_head();
                return oldHead ? oldHead->pData : std::shared_ptr<T>();
            }
            void push(T value)
            {
                std::shared_ptr<T> pNewData(std::make_shared<T>(std::move(value)));
                std::unique_ptr<node> p(new node);
                node* const pNewTail = p.get();
                std::lock_guard<std::mutex> lg(m_mutexTail);
                m_pTail->pData = pNewData;
                m_pTail->pNext = std::move(p);
                m_pTail = pNewTail;
            }
    };

    class ThreadPool
    {
        public:
            static ThreadPool& Get();
            //template<typename Callable, typename... Args> void Submit(Callable&& func, Args&&... args);
            template<typename FunctionType> void Submit(FunctionType f)
            {m_qWork.push(std::function<void()>(f));}

        private:
            ThreadPool();
            ~ThreadPool();

            void WorkerThread();

            std::atomic_bool m_bDone;
            ThreadSafeQueue<std::function<void()>> m_qWork;
            std::vector<std::thread> m_vThreads;
    };
};
