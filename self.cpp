#include "self.h"

using namespace std;


Self::Self(string sHostname, string sUrl,string sLabel, string sDescription) : Resource(sLabel, sDescription),
    m_sHostname(sHostname),
    m_sUrl(sUrl),
    m_nDnsVersion(0)
{

}

unsigned char Self::GetDnsVersion() const
{
    return m_nDnsVersion;
}

void Self::AddApiVersion(string sVersion)
{
    m_setVersion.insert(sVersion);
    UpdateVersionTime();
}

void Self::RemoveApiVersion(string sVersion)
{
    m_setVersion.erase(sVersion);
    UpdateVersionTime();
}

void Self::AddEndpoint(string sHost, unsigned int nPort, bool bSecure)
{
    m_setEndpoint.insert(endpoint(sHost, nPort, bSecure));
    UpdateVersionTime();
}

void Self::RemoveEndpoint(string sHost, unsigned int nPort)
{
    m_setEndpoint.erase(endpoint(sHost, nPort, false));
    UpdateVersionTime();
}

void Self::AddService(string sUrl, string sType)
{
    m_mService.insert(make_pair(sUrl, sType));
    UpdateVersionTime();
}

void Self::RemoveService(string sUrl)
{
    m_mService.erase(sUrl);
    UpdateVersionTime();
}

void Self::AddInterface(string sInterface, string sChassisMac, string sPortMac)
{
    m_mInterface.insert(make_pair(sInterface, interface(sChassisMac, sPortMac)));
    UpdateVersionTime();
}

void Self::RemoveInterface(string sInterface)
{
    m_mInterface.erase(sInterface);
    UpdateVersionTime();
}


void Self::AddInternalClock(string sName)
{
    m_mClock.insert(make_pair(sName, clock(clock::INTERNAL)));
    UpdateVersionTime();
}

void Self::AddPTPClock(string sName, bool bTraceable, string sVersion, string sGmid, bool bLocked)
{
    m_mClock.insert(make_pair(sName, clock(clock::PTP, sVersion, sGmid, bLocked, bTraceable)));
    UpdateVersionTime();
}

void Self::RemoveClock(string sName)
{
    m_mClock.erase(sName);
    UpdateVersionTime();
}

bool Self::Commit()
{
    if(Resource::Commit())
    {
        m_json["hostname"] = m_sHostname;
        m_json["href"] = m_sUrl;


        m_json["tags"] = Json::Value(Json::objectValue);
        m_json["caps"] = Json::Value(Json::objectValue);

        m_json["api"]["versions"] = Json::Value(Json::arrayValue);
        for(set<string>::iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
        {
            m_json["api"]["versions"].append(*itVersion);
        }

        m_json["api"]["endpoints"] = Json::Value(Json::arrayValue);
        for(set<endpoint>::iterator itEnd = m_setEndpoint.begin(); itEnd != m_setEndpoint.end(); ++itEnd)
        {

            m_json["api"]["endpoints"].append((*itEnd).Commit());
        }

        m_json["services"] = Json::Value(Json::arrayValue);
        for(map<string, string>::iterator itService = m_mService.begin(); itService != m_mService.end(); ++itService)
        {
            Json::Value jsService;
            jsService["href"] = itService->first;
            jsService["type"] = itService->first;
            m_json["services"].append(jsService);
        }

        m_json["clocks"] = Json::Value(Json::arrayValue);
        for(map<string, clock>::iterator itClock = m_mClock.begin(); itClock != m_mClock.end(); ++itClock)
        {
            Json::Value jsClock(itClock->second.Commit());
            jsClock["name"] = itClock->first;
            m_json["clocks"].append(jsClock);
        }

        m_json["interfaces"] = Json::Value(Json::arrayValue);
        for(map<string, interface>::iterator itInterface = m_mInterface.begin(); itInterface != m_mInterface.end(); ++itInterface)
        {
            Json::Value jsInterface;
            jsInterface["name"] = itInterface->first;
            jsInterface["chassis_id"] = itInterface->second.sChassisMac;
            jsInterface["port_id"] = itInterface->second.sPortMac;

            m_json["interfaces"].append(jsInterface);
        }

        if(m_nDnsVersion < 255)
        {
            m_nDnsVersion ++;
        }
        else
        {
            m_nDnsVersion = 0;
        }
        return true;
    }
    return false;
}

Json::Value Self::JsonVersions() const
{
    Json::Value jsVersion;
    for(set<string>::iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
    {
        jsVersion.append(*itVersion);
    }
    return jsVersion;
}


bool Self::IsVersionSupported(std::string sVersion) const
{
    return (m_setVersion.find(sVersion) != m_setVersion.end());
}
