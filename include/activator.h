#pragma once
#include <thread>
#include <condition_variable>
#include <map>
#include <memory>
#include <list>

namespace pml
{
    namespace nmos
    {
        class IOResource;
        class Sender;
        class Receiver;
        class NodeApiPrivate;

        class Activator
        {
            public:

                Activator(NodeApiPrivate& api);
                ~Activator();


                void Activate();

                bool ActivationsPending();
                bool Wait();
                void PrimeWait();

                bool IsRunning();


                void AddActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<pml::nmos::IOResource> pResource);
                void RemoveActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<pml::nmos::IOResource> pResource);

            private:




                NodeApiPrivate& m_api;

                bool m_bRunning;
                bool m_bWait;
                std::mutex m_mutex;
                std::condition_variable m_cvSync;

                std::map<std::chrono::time_point<std::chrono::high_resolution_clock>, std::shared_ptr<pml::nmos::IOResource> > m_mmActivations;

                std::list<std::unique_ptr<std::thread>> m_lstThreads;
        };
    };
};
