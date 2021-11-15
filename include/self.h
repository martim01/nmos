#pragma once
#include <list>
#include <set>
#include <map>
#include <string>
#include "resource.h"
#include "nmosdlldefine.h"
#include "version.h"

namespace pml
{
    namespace nmos
    {
        struct ifendpoint
        {
            ifendpoint(const std::string& sH, unsigned int nP, bool bP) : sHost(sH), nPort(nP), bSecure(bP){}
            bool operator<(const ifendpoint& endp) const
            {
                return (sHost < endp.sHost || (sHost == endp.sHost && nPort < endp.nPort));
            }
            bool operator==(const ifendpoint& endp) const
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

        struct nodeinterface
        {
            nodeinterface(const std::string& sC="", const std::string& sI="") : sChassisMac(sC), sPortMac(sI){}
            std::string sChassisMac;
            std::string sPortMac;
            std::string sMainIpAddress;
        };


        class NMOS_EXPOSE Self : public Resource
        {
            public:
                Self(const std::string& sHostname, const std::string& sUrl,const std::string& sLabel, const std::string& sDescription);
                Self() : Resource("node"){}
                bool UpdateFromJson(const Json::Value& jsData);
                ~Self();

                void Init(const std::string& sHostname, const std::string& sUrl,const std::string& sLabel, const std::string& sDescription);


                const std::set<ApiVersion>& GetApiVersions() const;

                void AddService(const std::string& sUrl, const std::string& sType);
                void RemoveService(const std::string& sUrl);

                void AddInterface(const std::string& sInterface, const std::string& sChassisMac="", const std::string& sPortMac="");
                void RemoveInterface(const std::string& sInterface);


                void AddInternalClock(const std::string& sName);
                void AddPTPClock(const std::string& sName, bool bTraceable, const std::string& sVersion, const std::string& sGmid, bool bLocked);
                void RemoveClock(const std::string& sName);

                std::string GetBestClock();

                void AddCap(){}
                void RemoveCap(){}

                bool IsVersionSupported(const std::string& sVersion) const;
                Json::Value JsonVersions() const;

                virtual bool Commit(const ApiVersion& version);
                bool Commit();

                unsigned char GetDnsVersion() const;

                std::string CreateClockSdp(const std::string& sClockName, const std::string& sInterface) const;

                std::set<ifendpoint>::const_iterator GetEndpointsBegin() const;
                std::set<ifendpoint>::const_iterator GetEndpointsEnd() const;

                std::map<std::string, nodeinterface>::const_iterator GetInterfaceBegin() const;
                std::map<std::string, nodeinterface>::const_iterator GetInterfaceEnd() const;
                std::map<std::string, nodeinterface>::const_iterator FindInterface(const std::string& sInterface) const;

                std::set<ApiVersion>::const_iterator GetApiVersionBegin() const;
                std::set<ApiVersion>::const_iterator GetApiVersionEnd() const;

                static void GetAddresses(const std::string& sInterface, nodeinterface& anInterface);

            protected:
                friend class NodeApiPrivate;

                void AddEndpoint(const std::string& sHost, unsigned int nPort, bool bSecure);
                void RemoveEndpoint(const std::string& sHost, unsigned int nPort);

            private:
                void AddApiVersion(unsigned short nMajor, unsigned short nMinor);
                void RemoveApiVersion(unsigned short nMajor, unsigned short nMinor);


                std::string m_sHostname;
                std::string m_sUrl;
                unsigned char m_nDnsVersion;






                struct clock
                {
                    clock(int nT, const std::string& sV="", const std::string& sG="", bool bL = true, bool bT=false ) : nType(nT), sVersion(sV), sGmid(sG), bLocked(bL), bTraceable(bT){}

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

                std::set<ApiVersion> m_setVersion;
                std::set<ifendpoint> m_setEndpoint;
                std::map<std::string, std::string> m_mService;
                std::map<std::string, nodeinterface> m_mInterface;
                std::map<std::string, clock> m_mClock;
                std::map<std::string, clock> m_mClockCommited;

        };
    };
};
