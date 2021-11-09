#include "activator.h"
#include "nodeapi.h"
#include "sendernode.h"
#include "receivernode.h"
#include "utils.h"

static void ActivationThread()
{
    while(Activator::Get().IsRunning() && Activator::Get().ActivationsPending())
    {
        Activator::Get().PrimeWait();
        if(Activator::Get().Wait() == true)
        {
            //activate stuff
            Activator::Get().Activate();
        }
        else
        {
            //reset time
        }
    }


}

Activator& Activator::Get()
{
    static Activator act;
    return act;
}

void Activator::AddActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<IOResource> pResource)
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

void Activator::RemoveActivation(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, std::shared_ptr<IOResource> pResource)
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

Activator::Activator() : m_bRunning(true), m_bWait(true)
{

}

Activator::~Activator()
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


void Activator::PrimeWait()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bWait = true;
}

bool Activator::Wait()
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


bool Activator::IsRunning()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bRunning;
}

bool Activator::ActivationsPending()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return (m_mmActivations.empty() == false);
}


void Activator::Activate()
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


bool Activator::AddActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pSender = NodeApi::Get().GetSender(sId);
    if(pSender)
    {
        AddActivation(tp, std::static_pointer_cast<IOResource>(pSender));
        return true;
    }
    return false;
}


bool Activator::RemoveActivationSender(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pSender = NodeApi::Get().GetSender(sId);
    if(pSender)
    {
        RemoveActivation(tp,std::static_pointer_cast<IOResource>(pSender));
        return true;
    }
    return false;
}

bool Activator::AddActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pReceiver = NodeApi::Get().GetReceiver(sId);
    if(pReceiver)
    {
        AddActivation(tp,std::static_pointer_cast<IOResource>(pReceiver));
        return true;
    }
    return false;
}

bool Activator::RemoveActivationReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sId)
{
    auto pReceiver = NodeApi::Get().GetReceiver(sId);
    if(pReceiver)
    {
        RemoveActivation(tp,std::static_pointer_cast<IOResource>(pReceiver));
        return true;
    }
    return false;
}
