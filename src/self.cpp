#include "self.h"
#ifdef __GNU__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#endif // __GNU__
#include <iostream>
#include <iomanip>
#include <sstream>
#include "log.h"


using namespace std;


Self::Self(string sHostname, string sUrl,string sLabel, string sDescription) : Resource("self", sLabel, sDescription),
    m_sHostname(sHostname),
    m_sUrl(sUrl),
    m_nDnsVersion(0)
{

}

Self::~Self()
{
}

void Self::Init(std::string sHostname, std::string sUrl,std::string sLabel, std::string sDescription)
{
    m_sHostname = sHostname;
    m_sUrl = sUrl;
    UpdateLabel(sLabel);
    UpdateDescription(sDescription);

    AddApiVersion(1,1);
    AddApiVersion(1,2);
}

unsigned char Self::GetDnsVersion() const
{
    return m_nDnsVersion;
}

void Self::AddApiVersion(unsigned short nMajor, unsigned short nMinor)
{
    m_setVersion.insert(ApiVersion(nMajor, nMinor));
}

void Self::RemoveApiVersion(unsigned short nMajor, unsigned short nMinor)
{
    m_setVersion.erase(ApiVersion(nMajor, nMinor));
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
    nodeinterface anInterface(sChassisMac, sPortMac);

    GetAddresses(sInterface, anInterface);

    m_mInterface.insert(make_pair(sInterface, anInterface));

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

bool Self::UpdateFromJson(const Json::Value& jsData)
{
    Resource::UpdateFromJson(jsData);
    m_bIsOk &= (jsData["href"].isString() && jsData["caps"].isObject() && jsData["api"].isObject() && jsData["services"].isArray() && jsData["clocks"].isArray() && jsData["interfaces"].isArray() && (jsData["hostname"].isString() || jsData["hostname"].empty()) && jsData["api"]["versions"].isArray() && jsData["api"]["endpoints"].isArray());

    if(m_bIsOk)
    {
        if(jsData["hostname"].isString())
        {
            m_sHostname = jsData["hostname"].asString();
        }
        m_sUrl = m_json["href"].asString();

        for(Json::ArrayIndex ai = 0; ai < jsData["api"]["versions"].size(); ++ai)
        {
            if(jsData["api"]["versions"][ai].isString() == false)
            {
                m_bIsOk = false;
                break;
            }
            else
            {
                m_setVersion.insert(jsData["api"]["versions"][ai].asString());
            }
        }

        for(Json::ArrayIndex ai = 0; ai < jsData["api"]["endpoints"].size(); ++ai)
        {
            if(jsData["api"]["endpoints"][ai].isObject() && jsData["api"]["endpoints"][ai]["host"].isString() && jsData["api"]["endpoints"][ai]["port"].isInt() && jsData["api"]["endpoints"][ai]["protocol"].isString())
            {
                m_setEndpoint.insert(endpoint(jsData["api"]["endpoints"][ai]["host"].asString(), jsData["api"]["endpoints"][ai]["port"].asInt(), (jsData["api"]["endpoints"][ai]["protocol"].asString() == "https")));
            }
            else
            {
                m_bIsOk = false;
                break;
            }
        }
    }

    return m_bIsOk;
}

bool Self::Commit()
{
    bool bChanged(false);
    for(std::set<ApiVersion>::const_iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
    {
        bChanged |= Commit(*itVersion);
    }
    return bChanged;
}

bool Self::Commit(const ApiVersion& version)
{
    if(Resource::Commit(version))
    {
        m_json["hostname"] = m_sHostname;
        m_json["href"] = m_sUrl;

        m_json["caps"] = Json::Value(Json::objectValue);

        m_json["api"]["versions"] = Json::Value(Json::arrayValue);
        for(set<ApiVersion>::iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
        {
            m_json["api"]["versions"].append((*itVersion).GetVersionAsString());
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
        m_mClockCommited = m_mClock;

        m_json["interfaces"] = Json::Value(Json::arrayValue);
        for(map<string, nodeinterface>::iterator itInterface = m_mInterface.begin(); itInterface != m_mInterface.end(); ++itInterface)
        {
            Json::Value jsInterface;
            jsInterface["name"] = itInterface->first;
            if(itInterface->second.sChassisMac.empty() == false)
            {
                jsInterface["chassis_id"] = itInterface->second.sChassisMac;
            }
            else
            {
                jsInterface["chassis_id"] = Json::nullValue;
            }
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
    for(set<ApiVersion>::iterator itVersion = m_setVersion.begin(); itVersion != m_setVersion.end(); ++itVersion)
    {
        jsVersion.append((*itVersion).GetVersionAsString());
    }
    return jsVersion;
}


bool Self::IsVersionSupported(std::string sVersion) const
{
    return (m_setVersion.find(ApiVersion(sVersion)) != m_setVersion.end());
}

const std::set<ApiVersion>& Self::GetApiVersions() const
{
    return m_setVersion;
}

//bool Self::StartServers()
//{
//    for(set<endpoint>::iterator itEndpoint = m_setEndpoint.begin(); itEndpoint != m_setEndpoint.end(); ++itEndpoint)
//    {
//        map<unsigned short, MicroServer*>::iterator itServer = m_mServers.find(itEndpoint->nPort);
//        if(itServer == m_mServers.end())
//        {
//            MicroServer* pServer(new MicroServer());
//            m_mServers.insert(make_pair(itEndpoint->nPort, pServer));
//            pServer->Init(itEndpoint->nPort);
//        }
//    }
//    return true;
//}


//bool Self::StopServers()
//{
//    for(map<unsigned short, MicroServer*>::iterator itServer = m_mServers.begin(); itServer != m_mServers.end(); ++itServer)
//    {
//        itServer->second->Stop();
//        delete itServer->second;
//    }
//    m_mServers.clear();
//    return true;
//}

set<endpoint>::const_iterator Self::GetEndpointsBegin() const
{
    return m_setEndpoint.begin();
}

set<endpoint>::const_iterator Self::GetEndpointsEnd() const
{
    return m_setEndpoint.end();
}

set<ApiVersion>::const_iterator Self::GetApiVersionBegin() const
{
    return m_setVersion.begin();
}

set<ApiVersion>::const_iterator Self::GetApiVersionEnd() const
{
    return m_setVersion.end();
}


void Self::GetAddresses(const std::string& sInterface, nodeinterface& anInterface)
{
    #ifdef __GNU__
    int fd = socket(AF_INET, SOCK_DGRAM,0);
    ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy((char*)ifr.ifr_ifrn.ifrn_name, sInterface.c_str(), IFNAMSIZ-1);
    ioctl(fd, SIOCGIFHWADDR, &ifr);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);


    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[0])&0xFF);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[1])&0xFF);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[2])&0xFF);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[3])&0xFF);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[4])&0xFF);
    ss << "-";
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (((int)ifr.ifr_hwaddr.sa_data[5])&0xFF);

    if(anInterface.sPortMac.empty())
    {
        anInterface.sPortMac = ss.str();
    }
    anInterface.sMainIpAddress = inet_ntoa((((sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    #else
    // @todo windows get mac address
    #endif
}


std::string Self::CreateClockSdp(std::string sInterface) const
{
    std::stringstream ss;
    //run through the clocks twice - first try to find a PTP clock. If none found then return the first internal one
    for(std::map<std::string, clock>::const_iterator itClock = m_mClockCommited.begin(); itClock != m_mClockCommited.end(); ++itClock)
    {
        if(itClock->second.nType == clock::PTP)
        {
            ss << "a=ts-refclk:ptp=";
            if(itClock->second.bTraceable)
            {
                ss << "traceable\r\n";
            }
            else
            {
                ss << itClock->second.sVersion << ":" << itClock->second.sGmid << "\r\n";
            }
            return ss.str();
        }
    }

    for(std::map<std::string, clock>::const_iterator itClock = m_mClockCommited.begin(); itClock != m_mClockCommited.end(); ++itClock)
    {
        if(itClock->second.nType == clock::INTERNAL)
        {
            map<std::string, nodeinterface>::const_iterator itInterface = m_mInterface.find(sInterface);
            if(itInterface != m_mInterface.end())
            {
                ss << "a=ts-refclk:localmac=" << itInterface->second.sPortMac << "\r\n";
            }
            else if(m_mInterface.empty())
            {
                ss << "a=ts-refclk:localmac=" << m_mInterface.begin()->second.sPortMac << "\r\n";
            }
            return ss.str();
        }
    }
    return ss.str();
}


std::map<std::string, nodeinterface>::const_iterator Self::GetInterfaceBegin() const
{
    return m_mInterface.begin();
}
std::map<std::string, nodeinterface>::const_iterator Self::GetInterfaceEnd() const
{
    return m_mInterface.end();
}
std::map<std::string, nodeinterface>::const_iterator Self::FindInterface(const std::string& sInterface) const
{
    return m_mInterface.find(sInterface);
}
