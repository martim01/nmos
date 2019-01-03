#include "nmosthread.h"
#include <thread>
#include <chrono>
#include "nodeapi.h"
#include "eventposter.h"
#include "log.h"

using namespace std;

void NmosThread::Main()
{

    //start the discovery and connection servers
    NodeApi::Get().StartHttpServers();

    //start the DNS-SD publisher server
    NodeApi::Get().StartmDNSServer();


    do
    {
        //browse for an registry server this will now wait until all services have been browsed for,
        NodeApi::Get().BrowseForRegistrationNode();

        //try to find the registartion node
        if(NodeApi::Get().FindRegistrationNode())
        {
            //remove the ver_ txt from our publisher
            NodeApi::Get().ModifyTxtRecords();

            //Run the registered operation loop
            if(RegisteredOperation())
            {   //exited cleanly so still registered
                NodeApi::Get().UnregisterSimple();
            }
        }
        else
        {
            Log::Get() << "Try again in 30s..." << std::endl;
            for(int i = 30; i > 0 && NodeApi::Get().IsRunning(); i--)
            {   //loop so we wake up to check if the thread should close...
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }while(NodeApi::Get().IsRunning());

}

bool NmosThread::RegisteredOperation()
{
    if(NodeApi::Get().RegisterSimple() == NodeApi::REG_DONE)
    {
        while(NodeApi::Get().IsRunning())
        {
            if(NodeApi::Get().WaitForCommit(5000))    //@todo we should be able to set heartbeat time
            {
                NodeApi::Get().UpdateRegisterSimple();
            }
            long nResponse = NodeApi::Get().RegistrationHeartbeat();
            if(nResponse == 0 || nResponse >= 500)
            {
                Log::Get() << "Registration server gone" << endl;
                return false;
            }
            else if(nResponse == 404)
            {
                if(NodeApi::Get().RegisterSimple() != NodeApi::REG_DONE)
                {
                    return false;
                }
            }
        }

        return true;
    }
    return false;
}