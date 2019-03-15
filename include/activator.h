#pragma once
#include <thread>
#include <condition_variable>
#include <map>
#include <memory>
#include "ioresource.h"

class Sender;
class Receiver;


class Activator
{
    public:
        static Activator& Get();

        bool AddActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId);
        bool RemoveActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId);

        bool AddActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId);
        bool RemoveActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId);

        void Activate();

        bool ActivationsPending();
        bool Wait();
        void PrimeWait();

        bool IsRunning();
    private:

        void AddActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<IOResource> pResource);
        void RemoveActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<IOResource> pResource);

        Activator();
        ~Activator();

        bool m_bRunning;
        bool m_bWait;
        std::mutex m_mutex;
        std::condition_variable m_cvSync;

        std::map<std::chrono::time_point<std::chrono::high_resolution_clock>, std::shared_ptr<IOResource> > m_mmActivations;
};

