#pragma once
#include <list>
#include <set>
#include <map>
#include <string>
#include "resource.h"
#include "dlldefine.h"



struct endpoint
{
    endpoint(std::string sH, unsigned int nP, bool bP) : sHost(sH), nPort(nP), bSecure(bP){}
    bool operator<(const endpoint& endp) const
    {
        return (sHost < endp.sHost || (sHost == endp.sHost && nPort < endp.nPort));
    }
    bool operator==(const endpoint& endp) const
    {
        return (sHost==endp.sHost && nPort == endp.nPort);
    }

    Json::Value Commit() const
    {
        Json::Value jsonEnd;
        jsonEnd["host"] = sHost;
        jsonEnd["port"] = nPort;
        if(bSecure)
        {
            jsonEnd["protocol"] = "https";
        }
        else
        {
            jsonEnd["protocol"] = "http";
        }
        return jsonEnd;
    }

    std::string sHost;
    unsigned int nPort;
    bool bSecure;
};

struct interface
{
    interface(std::string sC, std::string sI) : sChassisMac(sC), sPortMac(sI){}
    std::string sChassisMac;
    std::string sPortMac;
    std::string sMainIpAddress;
};


class NMOS_EXPOSE Self : public Resource
{
    public:
        Self(std::string sHostname, std::string sUrl,std::string sLabel, std::string sDescription);
        Self() : Resource("node"){}
        bool UpdateFromJson(const Json::Value& jsData);
        ~Self();

        void Init(std::string sHostname, std::string sUrl,std::string sLabel, std::string sDescription);

        void AddApiVersion(std::string sVersion);
        void RemoveApiVersion(std::string sVersion);



        void AddService(std::string sUrl, std::string sType);
        void RemoveService(std::string sUrl);

        void AddInterface(std::string sInterface, std::string sChassisMac="", std::string sPortMac="");
        void RemoveInterface(std::string sInterface);


        void AddInternalClock(std::string sName);
        void AddPTPClock(std::string sName, bool bTraceable, std::string sVersion, std::string sGmid, bool bLocked);
        void RemoveClock(std::string sName);

        void AddCap(){}
        void RemoveCap(){}

        bool IsVersionSupported(std::string sVersion) const;
        Json::Value JsonVersions() const;

        virtual bool Commit();

        unsigned char GetDnsVersion() const;

        std::string CreateClockSdp(std::string sInterface) const;

        std::set<endpoint>::const_iterator GetEndpointsBegin() const;
        std::set<endpoint>::const_iterator GetEndpointsEnd() const;

        std::map<std::string, interface>::const_iterator GetInterfaceBegin() const;
        std::map<std::string, interface>::const_iterator GetInterfaceEnd() const;
        std::map<std::string, interface>::const_iterator FindInterface(const std::string& sInterface) const;

    protected:
        friend class NodeApi;

        void AddEndpoint(std::string sHost, unsigned int nPort, bool bSecure);
        void RemoveEndpoint(std::string sHost, unsigned int nPort);

    private:
        std::string m_sHostname;
        std::string m_sUrl;
        unsigned char m_nDnsVersion;




        void GetAddresses(const std::string& sInterface, interface& anInterface);

        struct clock
        {
            clock(int nT, std::string sV="", std::string sG="", bool bL = true, bool bT=false ) : nType(nT), sVersion(sV), sGmid(sG), bLocked(bL), bTraceable(bT){}

            Json::Value Commit()
            {
                Json::Value jsClock;
                if(nType == INTERNAL)
                {
                    jsClock["ref_type"] = "internal";
                }
                else
                {
                    jsClock["ref_type"] = "ptp";
                    jsClock["traceable"] = bTraceable;
                    jsClock["version"] = sVersion;
                    jsClock["gmid"] = sGmid;
                    jsClock["locked"] = bLocked;
                }
                return jsClock;
            }

            int nType;
            std::string sVersion;
            std::string sGmid;
            bool bLocked;
            bool bTraceable;

            enum {INTERNAL, PTP};
        };

        std::set<std::string> m_setVersion;
        std::set<endpoint> m_setEndpoint;
        std::map<std::string, std::string> m_mService;
        std::map<std::string, interface> m_mInterface;
        std::map<std::string, clock> m_mClock;
        std::map<std::string, clock> m_mClockCommited;



};
