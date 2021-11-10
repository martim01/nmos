#include "receiver.h"
#include "nodeapi.h"
#include "activator.h"
#include "utils.h"
#include "log.h"

using namespace pml::nmos;

Receiver::Receiver(const std::string& sLabel, const std::string& sDescription, enumTransport eTransport, const std::string& sDeviceId, enumType eFormat, int flagsTransport) :
ReceiverBase(sLabel, sDescription, eTransport, sDeviceId, eFormat, flagsTransport)
{

}

Receiver::Receiver() : ReceiverBase()
{
}


void Receiver::CommitActivation()
{
    //reset the staged activation
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

    NodeApi::Get().ReceiverActivated(GetId());
}

bool Receiver::Stage(const connectionReceiver& conRequest)
{
    bool bOk(true);
    m_mutex.lock();

    m_Staged = conRequest;

    m_mutex.unlock();

    switch(m_Staged.eActivate)
    {
        case connection::ACT_NULL:
            if(m_Staged.sActivationTime.empty() == false)
            {
                Activator::Get().RemoveActivationReceiver(m_Staged.tpActivation, GetId());
                m_Staged.sActivationTime.clear();
                m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
            }
            m_bActivateAllowed = false;
            break;
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTaiTime();
            m_bActivateAllowed = true;
            Activate(true);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(m_Staged.sRequestedTime, tp))
                {
                    m_bActivateAllowed = true;
                    m_Staged.sActivationTime = m_Staged.sRequestedTime;
                    m_Staged.tpActivation = tp;
                    Activator::Get().AddActivationReceiver(tp, GetId());
                }
                else
                {
                    bOk = false;
                }
            }
            break;
        case connection::ACT_RELATIVE:
            try
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp(std::chrono::high_resolution_clock::now().time_since_epoch());
                std::vector<std::string> vTime;
                SplitString(vTime, m_Staged.sRequestedTime, ':');
                if(vTime.size() != 2)
                {
                    throw std::invalid_argument("invalid time");
                }

                m_bActivateAllowed = true;
                std::chrono::nanoseconds nano((static_cast<long long int>(std::stoul(vTime[0]))*1000000000)+stoul(vTime[1]));
                std::chrono::time_point<std::chrono::high_resolution_clock> tpAbs(tp+nano-LEAP_SECONDS);


                m_Staged.sActivationTime = ConvertTimeToString(tpAbs, true);
                m_Staged.tpActivation = tpAbs;
                Activator::Get().AddActivationReceiver(tpAbs, GetId());
            }
            catch(std::invalid_argument& ia)
            {
                m_bActivateAllowed = false;
                bOk = false;
            }
            break;
        default:
            pmlLog(pml::LOG_ERROR) << "Unexpected patch" ;
    }
    return bOk;
}

void Receiver::Activate(bool bImmediate)
{
    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;


    //Change auto settings to what they actually are
    m_Active.tpReceiver.Actualize(m_sInterfaceIp);

    //activeate - set subscription, receiverId and active on master_enable. Commit afterwards
    if(m_Active.bMasterEnable)
    {
        m_sSenderId = m_Staged.sSenderId;
        m_bSenderActive = m_Staged.bMasterEnable;
    }
    else
    {
        m_sSenderId.clear();
        m_bSenderActive = false;
    }

    if(!bImmediate)
    {
        CommitActivation();
    }
}

