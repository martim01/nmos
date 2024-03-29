#pragma once
#include "resource.h"
#include <array>
#include <set>
#include <string>
#include "constraint.h"
#include <optional>

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;
        class IOResource : public Resource
        {
            public:
                enum enumTransport {RTP=0, RTP_UCAST, RTP_MCAST, DASH, MQTT, WEBSOCKET, UNKNOWN_TRANSPORT};

                IOResource(const std::string& sType, const std::string& sLabel, const std::string& sDescription, enumTransport eTransport) : Resource(sType, sLabel, sDescription), m_eTransport(eTransport){}
                IOResource(const std::string& sType) : Resource(sType){}

                virtual void Activate(bool bImmediate, NodeApiPrivate& api){};

                std::string GetTransportType() const
                {
                    if(m_eTransport < STR_TRANSPORT.size())
                    {
                        return STR_TRANSPORT[m_eTransport];
                    }
                    else
                    {
                        return "Unknown";
                    }
                }
                void SetTransport(enumTransport eTransport);

                void AddInterfaceBinding(const std::string& sInterface);
                void RemoveInterfaceBinding(const std::string& sInterface);

                bool AddConstraint(const std::string& sKey, const std::optional<int>& minValue, const std::optional<int>& maxValue, const std::optional<std::string>& pattern,
                                   const std::vector<pairEnum_t>& vEnum, const std::optional<size_t>& tp);

                bool ClearConstraint(const std::string& sKey, const std::optional<size_t>& tp);

                Json::Value GetConnectionConstraintsJson(const ApiVersion& version) const;

                const std::vector<std::string>& GetInterfaces() const { return m_vInterfaces;}


            protected:
                friend class NodeApiPrivate;

                bool CheckConstraints(const Json::Value& jsonRequest);
                bool CheckConstraints(const Json::Value& jsonRequest, const Constraints& con);

                void CreateConstraints(const Json::Value& jsonStaged);

                enumTransport m_eTransport;
                std::vector<std::string> m_vInterfaces;

                std::vector<Constraints> m_vConstraints;

                static const std::array<std::string,6> STR_TRANSPORT;
        };
    };
};
