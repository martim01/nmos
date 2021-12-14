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


void Receiver::CommitActivation()
{
    m_Staged.eActivate = connection::ACT_NULL;
    m_Staged.sActivationTime.clear();
    m_Staged.sRequestedTime.clear();

    UpdateVersionTime();

}

connection::enumActivate Receiver::Stage(const connectionReceiver& conRequest)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_Staged = conRequest;
    return m_Staged.eActivate;
}

void Receiver::Activate()
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


}

void Receiver::SetSender(const std::string& sSenderId, const std::string& sSdp, const std::string& sInterfaceIp)
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
    }
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


void Receiver::RemoveStagedActivationTime()
{
    m_Staged.sActivationTime.clear();
    m_Staged.tpActivation = std::chrono::time_point<std::chrono::high_resolution_clock>();
}

void Receiver::SetStagedActivationTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp)
{
    m_Staged.tpActivation = tp;
    m_Staged.sActivationTime = ConvertTimeToString(tp);
}
