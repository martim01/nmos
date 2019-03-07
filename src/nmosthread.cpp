#include "nmosthread.h"
#include <thread>
#include <chrono>
#include "nodeapi.h"
#include "eventposter.h"
#include "log.h"

using namespace std;

void NodeThread::Main()
{
    //start the discovery and connection servers
    NodeApi::Get().StartHttpServers();
    //start the DNS-SD publisher server
    NodeApi::Get().StartmDNSServer();

    //browse for an registry server this will now wait until all services have been browsed for,

    NodeApi::Get().BrowseForRegistrationNode();
    bool bDoneOnce(false);
    int nBackoff = 1000;
    do
    {
        if(NodeApi::Get().Wait(nBackoff))
        {
            switch(NodeApi::Get().GetSignal())
            {
                case NodeApi::SIG_BROWSE_DONE:
                    FindRegisterNode();
                    break;
            }
        }
        else
        {
            FindRegisterNode();
            nBackoff+= 500;
        }
    }while(NodeApi::Get().IsRunning());

}

bool NodeThread::FindRegisterNode()
{
    //try to find the registartion node
    while(NodeApi::Get().FindRegistrationNode())
    {
        //remove the ver_ txt from our publisher
        NodeApi::Get().ModifyTxtRecords();

        //Run the registered operation loop
        if(RegisteredOperation(ApiVersion(1,2)))    //@todo use the version that the registry wants us to
        {   //exited cleanly so still registered
            NodeApi::Get().UnregisterSimple();
            break;  //break out of the while loop as we are exiting
        }

    }

}

bool NodeThread::RegisteredOperation(const ApiVersion& version)
{
    if(NodeApi::Get().RegisterSimple(version) == NodeApi::REG_DONE)
    {
        long nResponse = NodeApi::Get().RegistrationHeartbeat();

        while(NodeApi::Get().IsRunning())
        {
            if(NodeApi::Get().Wait(NodeApi::Get().GetHeartbeatTime()))
            {
                switch(NodeApi::Get().GetSignal())
                {
                    case NodeApi::SIG_COMMIT:
                        NodeApi::Get().UpdateRegisterSimple(version);
                        break;
                    case NodeApi::SIG_INSTANCE_REMOVED: //this means our reg node has gone
                        Log::Get() << "Registration server gone" << endl;
                        return false;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                long nResponse = NodeApi::Get().RegistrationHeartbeat();
                if(nResponse == 0 || nResponse >= 500)
                {
                    Log::Get() << "Registration server gone" << endl;
                    return false;
                }
                else if(nResponse == 404)
                {
                    if(NodeApi::Get().RegisterSimple(version) != NodeApi::REG_DONE)
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }
    return false;
}

