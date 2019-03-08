#pragma once

#include <string>
#include <map>
#include <set>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata);
static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata);

class ServicePublisher
{
    public:
        ServicePublisher(std::string sName, std::string sService, unsigned short nPort, std::string sHostname);
        ~ServicePublisher();

        bool Start();
        void Stop();
        void Modify();

        void AddTxt(std::string sKey, std::string sValue, bool bModify);
        void RemoveTxt(std::string sKey,bool bModify);

        void EntryGroupCallback(AvahiEntryGroup* pGroup, AvahiEntryGroupState state);
        void ClientCallback(AvahiClient* pClient, AvahiClientState state);

    private:

        void CreateServices();
        void Collision();

        void ThreadQuit();

        AvahiStringList* GetTxtList();

        AvahiClient* m_pClient;
        AvahiEntryGroup* m_pGroup;
        AvahiThreadedPoll* m_pThreadedPoll;

        std::string m_sName;
        std::string m_sService;
        unsigned short m_nPort;
        std::string m_sHostname;
        char* m_psName;

        std::map<std::string, std::string> m_mTxt;
        //"_nmos-node._tcp"
};

