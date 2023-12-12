#include "nmosthread.h"
#include <thread>
#include <chrono>
#include "nodeapiprivate.h"
#include "eventposter.h"
#include "log.h"

using namespace pml::nmos;

void NodeApiPrivate::Run()
{
    //start the discovery and connection servers
    StartHttpServers();
    //start the DNS-SD publisher server
    StartmDNSPublisher();

    //browse for an registry server this will now wait until all services have been browsed for,

    BrowseForRegistrationNode();
//    bool bDoneOnce(false);
    int nBackoff = 5000;
    do
    {
        if(Wait(nBackoff))
        {
            switch(GetSignal())
            {
                case NodeApiPrivate::SIG_BROWSE_DONE:
                    FindRegisterNode();
                    break;
                default:
                break;
            }
        }
        else
        {
            FindRegisterNode();
            nBackoff+= 500;
        }
    }while(IsRunning());

    //now stop the discovery and connection servers
    StopHttpServers();
    //and the dns-sd publisher
    StopmDNSPublisher();
}

bool NodeApiPrivate::FindRegisterNode()
{
    //try to find the registartion node
    while(FindRegistrationNode())
    {
        //remove the ver_ txt from our p9ublisher
        ModifyTxtRecords();

        //Run the registered operation loop
        if(RegisteredOperation())
        {   //exited cleanly so still registered
            UnregisterSimple();
            break;  //break out of the while loop as we are exiting
        }

    }
    return false;

}

bool NodeApiPrivate::RegisteredOperation()
{
    if(RegisterSimple() == NodeApiPrivate::REG_DONE)
    {
        if(HandleHeartbeatResponse(RegistrationHeartbeat()) == false)
        {
            return false;
        }

        while(IsRunning())
        {
            if(WaitUntil(GetHeartbeatTime()))
            {
                switch(GetSignal())
                {
                    //case NodeApiPrivate::SIG_COMMIT:
                    //    UpdateRegisterSimple();
                    //    break;
                    case NodeApiPrivate::SIG_INSTANCE_REMOVED: //this means our reg node has gone
                        pmlLog(pml::LOG_INFO, "pml::nmos") << "NMOS: " << "Registration server gone" ;
                        m_nRegistrationStatus = REG_FAILED;
                        PostRegisterStatus();
                        return false;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                if(HandleHeartbeatResponse(RegistrationHeartbeat()) == false)
                {
                    return false;
                }
            }
        }

        return true;
    }
    return false;
}

bool NodeApiPrivate::HandleHeartbeatResponse(unsigned int nResponse)
{
    bool bRegistryOk(true);
    if(nResponse == 0 || nResponse >= 500)
    {   //registry error
        pmlLog(pml::LOG_WARN, "pml::nmos") << "NMOS: " << "Registration server gone" ;
        bRegistryOk = false;
        m_nRegistrationStatus = REG_FAILED;
        PostRegisterStatus();
    }
    else if(nResponse == 404)
    { //not known about re-register
        pmlLog(pml::LOG_WARN, "pml::nmos") << "NMOS: " << "Registration server forgotten node. Reregister" ;
        m_nRegistrationStatus = REG_FAILED;
        PostRegisterStatus();

        if(RegisterSimple() != NodeApiPrivate::REG_DONE)
        {
            bRegistryOk = false;
            m_nRegistrationStatus = REG_FAILED;
            PostRegisterStatus();
        }
    }
    else if(nResponse == 409)
    {  //already registered with different  - deregister and re-register
       pmlLog(pml::LOG_WARN, "pml::nmos") << "NMOS: " << "Registered with different . Deregister and reregister." ;
       UnregisterSimple();
       if(RegisterSimple() != NodeApiPrivate::REG_DONE)
       {
            bRegistryOk = false;
            m_nRegistrationStatus = REG_FAILED;
            PostRegisterStatus();
       }
    }
    else if(nResponse >= 400)
    { //probably validation failure
        pmlLog(pml::LOG_ERROR, "pml::nmos") << "NMOS: " << "Registry validation error!";
        bRegistryOk = false;
        m_nRegistrationStatus = REG_FAILED;
        PostRegisterStatus();
    }
    return bRegistryOk;
}
