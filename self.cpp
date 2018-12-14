#include "self.h"

using namespace std;


Self::Self(string sHostname, string sUrl,string sLabel, string sDescription) : Resource(sLabel, sDescription),
    m_sHostname(sHostname),
    m_sUrl(sUrl),
    m_nVersion(0)
{

}


void Self::AddVersion(string sVersion)
{
    m_setVersion.insert(sVersion);
}

void Self::RemoveVersion(string sVersion)
{
    m_setVersion.erase(sVersion);
}

void Self::AddEndpoint(string sHost, unsigned int nPort, bool bSecure)
{
    m_setEndpoint.insert(endpoint(sHost, nPort, bSecure));
}

void Self::RemoveEndpoint(string sHost, unsigned int nPort)
{
    m_setEndpoint.erase(endpoint(sHost, nPort, false));
}

void Self::AddService(string sUrl, string sType)
{
    m_mService.insert(make_pair(sUrl, sType));
}

void Self::RemoveService(string sUrl)
{
    m_mService.erase(sUrl);
}

void Self::AddInterface(string sInterface, string sChassisMac, string sPortMac)
{
    m_mInterface.insert(make_pair(sInterface, interface(sChassisMac, sPortMac)));
}

void Self::RemoveInterface(string sInterface)
{
    m_mInterface.erase(sInterface);
}


void Self::AddInternalClock(string sName)
{
    m_mClock.insert(make_pair(sName, clock(clock::INTERNAL)));
}

void Self::AddPTPClock(string sName, bool bTraceable, string sVersion, string sGmid, bool bLocked)
{
    m_mClock.insert(make_pair(sName, clock(clock::PTP, sVersion, sGmid, bLocked, bTraceable)));
}

void Self::RemoveClock(string sName)
{
    m_mClock.erase(sName);
}

Json::Value Self::ToJson()  const
{

    Json::Value jsonSelf(Resource::ToJson());

    jsonSelf["hostname"] = m_sHostname;
    jsonSelf["href"] = m_sUrl;


    jsonSelf["tags"] = Json::Value(Json::objectValue);
    jsonSelf["caps"] = Json::Value(Json::objectValue);

    jsonSelf["api"]["version"] = Json::Value(Json::arrayValue);
    for(set<string>::iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
    {
        jsonSelf["api"]["version"].append(*itVersion);
    }

    jsonSelf["api"]["endpoints"] = Json::Value(Json::arrayValue);
    for(set<endpoint>::iterator itEnd = m_setEndpoint.begin(); itEnd != m_setEndpoint.end(); ++itEnd)
    {
        jsonSelf["api"]["endpoints"].append((*itEnd).ToJson());
    }

    jsonSelf["services"] = Json::Value(Json::arrayValue);
    for(map<string, string>::const_iterator itService = m_mService.begin(); itService != m_mService.end(); ++itService)
    {
        Json::Value jsService;
        jsService["href"] = itService->first;
        jsService["type"] = itService->first;
        jsonSelf["services"].append(jsService);
    }

    jsonSelf["clocks"] = Json::Value(Json::arrayValue);
    for(map<string, clock>::const_iterator itClock = m_mClock.begin(); itClock != m_mClock.end(); ++itClock)
    {
        Json::Value jsClock(itClock->second.ToJson());
        jsClock["name"] = itClock->first;
        jsonSelf["clocks"].append(jsClock);
    }

    jsonSelf["interfaces"] = Json::Value(Json::arrayValue);
    for(map<string, interface>::const_iterator itInterface = m_mInterface.begin(); itInterface != m_mInterface.end(); ++itInterface)
    {
        Json::Value jsInterface;
        jsInterface["name"] = itInterface->first;
        jsInterface["chassis_id"] = itInterface->second.sChassisMac;
        jsInterface["port_id"] = itInterface->second.sPortMac;

        jsonSelf["interfaces"].append(jsInterface);
    }

    return jsonSelf;
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
