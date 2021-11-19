#include "receiver.h"
#include "nodeapiprivate.h"
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


void Receiver::CommitActivation(NodeApiPrivate& api)
{
    //reset the staged activation
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

    api.ReceiverActivated(GetId());
}

bool Receiver::Stage(const connectionReceiver& conRequest, NodeApiPrivate& api)
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
                api.RemoveActivationReceiver(m_Staged.tpActivation, GetId());
                m_Staged.sActivationTime.clear();
                m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
            }
            m_bActivateAllowed = false;
            break;
        case connection::ACT_NOW:
            m_Staged.sActivationTime = GetCurrentTaiTime();
            m_bActivateAllowed = true;
            Activate(true, api);
            break;
        case connection::ACT_ABSOLUTE:
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> tp;
                if(ConvertTaiStringToTimePoint(m_Staged.sRequestedTime, tp))
                {
                    m_bActivateAllowed = true;
                    m_Staged.sActivationTime = m_Staged.sRequestedTime;
                    m_Staged.tpActivation = tp;
                    api.AddActivationReceiver(tp, GetId());
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
                api.AddActivationReceiver(tpAbs, GetId());
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

void Receiver::Activate(bool bImmediate, NodeApiPrivate& api)
{
    m_bActivateAllowed = false;
    //move the staged parameters to active parameters
    m_Active = m_Staged;


    //Change auto settings to what they actually are
    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpReceivers[i].Actualize(m_sInterfaceIp);
    }

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
        CommitActivation(api);
    }
}

void Receiver::SetSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp, NodeApiPrivate& api)
{
    if(sSenderId.empty() == false)
    {
        pmlLog(pml::LOG_INFO) << "Receiver subscribe to sender " << sSenderId ;
        m_sSenderId = sSenderId;

        // need to update the IS-05 stage and active connection settings to match
        m_Staged.sSenderId = sSenderId;
        m_Staged.bMasterEnable = true;
        m_Staged.sTransportFileData = sSdp;
        m_Staged.sTransportFileType = "application/sdp";
        m_Staged.sActivationTime = GetCurrentTaiTime();
        SetupActivation(sInterfaceIp);
        Activate(false, api);
    }
    else
    {   //this means unsubscribe
        pmlLog(pml::LOG_INFO) << "Receiver unssubscribe " ;
        m_sSenderId.clear();

        // need to update the IS-05 stage and active connection settings to match
        m_Staged.sSenderId.clear();
        m_Staged.bMasterEnable = false;
        m_Staged.sTransportFileData.clear();
        m_Staged.sTransportFileType.clear();
        m_Staged.sActivationTime = GetCurrentTaiTime();
        SetupActivation(sInterfaceIp);
        Activate(false, api);
    }
    UpdateVersionTime();

}

void Receiver::MasterEnable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_Active.bMasterEnable = bEnable;
    m_Staged.bMasterEnable = bEnable;

    for(size_t i = 0;i < m_vConstraints.size(); i++)
    {
        m_Active.tpReceivers[i].bRtpEnabled = bEnable;
        m_Staged.tpReceivers[i].bRtpEnabled = bEnable;
    }
    UpdateVersionTime();
}

void Receiver::SetupActivation(const std::string& sInterfaceIp)
{
    m_sInterfaceIp = sInterfaceIp;
}
