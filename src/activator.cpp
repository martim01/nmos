#include "activator.h"
#include "nodeapi.h"
#include "sender.h"
#include "receiver.h"
#include "utils.h"
#include "ioresource.h"


static void ActivationThread()
{
    while(pml::nmos::Activator::Get().IsRunning() && pml::nmos::Activator::Get().ActivationsPending())
    {
        pml::nmos::Activator::Get().PrimeWait();
        if(pml::nmos::Activator::Get().Wait() == true)
        {
            //activate stuff
            pml::nmos::Activator::Get().Activate();
        }
        else
        {
            //reset time
        }
    }


}

pml::nmos::Activator& pml::nmos::Activator::Get()
{
    static pml::nmos::Activator act;
    return act;
}

void pml::nmos::Activator::AddActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<pml::nmos::IOResource> pResource)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_mmActivations.empty() == false)
    {
        if(m_mmActivations.begin()->first > tp)
        {//we are adding an activation sooner than the one currently waited on
            //insert the new activation
            m_mmActivations.insert(make_pair(tp,pResource));

            //wake the thread up to change the time
            m_bWait = false;
            m_cvSync.notify_one();
        }
        else
        {
            //insert the new activation
            m_mmActivations.insert(make_pair(tp,pResource));
        }
    }
    else
    {
        //insert the new activation
        m_mmActivations.insert(make_pair(tp,pResource));

        //create the thread
        m_lstThreads.push_back(std::make_unique<std::thread>(ActivationThread));
    }
}

void pml::nmos::Activator::RemoveActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<pml::nmos::IOResource> pResource)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_mmActivations.empty() == false)
    {
        if(m_mmActivations.begin()->first == tp && m_mmActivations.count(tp) == 1)
        { //we are removing the event that is queued to activate next
            m_mmActivations.erase(m_mmActivations.begin());

            //wake the thread up to change the time
            m_bWait = false;
            m_cvSync.notify_one();
        }
        else
        {   //don't need to disturb the thread
            for(auto itEvent = m_mmActivations.lower_bound(tp); itEvent != m_mmActivations.upper_bound(tp); ++itEvent)
            {
                if(itEvent->second == pResource)
                {
                    m_mmActivations.erase(itEvent);
                    break;
                }
            }
        }
    }
}

pml::nmos::Activator::Activator() : m_bRunning(true), m_bWait(true)
{

}

pml::nmos::Activator::~Activator()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bRunning = false;
    m_bWait = false;
    m_cvSync.notify_one();

    for(const auto& pThread : m_lstThreads)
    {
        pThread->join();
    }
    m_lstThreads.clear();
}


void pml::nmos::Activator::PrimeWait()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bWait = true;
}

bool pml::nmos::Activator::Wait()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto tp  = m_mmActivations.begin()->first;
    tp-=LEAP_SECONDS;
    while(m_bWait == true)
    {
        if(m_cvSync.wait_until(lk, tp) == std::cv_status::timeout)
        {   //waited until activation time
            return true;
        }
    }
    return false;   //signalled
}


bool pml::nmos::Activator::IsRunning()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bRunning;
}

bool pml::nmos::Activator::ActivationsPending()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return (m_mmActivations.empty() == false);
}


void pml::nmos::Activator::Activate()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_mmActivations.empty() == false)
    {
        for(auto itEvent = m_mmActivations.begin(); itEvent != m_mmActivations.upper_bound(m_mmActivations.begin()->first); ++itEvent)
        {
            itEvent->second->Activate();
        }
        m_mmActivations.erase(m_mmActivations.begin()->first);
    }
}


bool pml::nmos::Activator::AddActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pSender = NodeApi::Get().GetSender(sId);
    if(pSender)
    {
        AddActivation(tp, std::static_pointer_cast<pml::nmos::IOResource>(pSender));
        return true;
    }
    return false;
}


bool pml::nmos::Activator::RemoveActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pSender = NodeApi::Get().GetSender(sId);
    if(pSender)
    {
        RemoveActivation(tp,std::static_pointer_cast<pml::nmos::IOResource>(pSender));
        return true;
    }
    return false;
}

bool pml::nmos::Activator::AddActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pReceiver = NodeApi::Get().GetReceiver(sId);
    if(pReceiver)
    {
        AddActivation(tp,std::static_pointer_cast<pml::nmos::IOResource>(pReceiver));
        return true;
    }
    return false;
}

bool pml::nmos::Activator::RemoveActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pReceiver = NodeApi::Get().GetReceiver(sId);
    if(pReceiver)
    {
        RemoveActivation(tp,std::static_pointer_cast<pml::nmos::IOResource>(pReceiver));
        return true;
    }
    return false;
}
