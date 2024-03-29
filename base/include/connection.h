#pragma once
#include "nmosapiversion.h"
#include "transportparams.h"
#include "nmosdlldefine.h"
#include <chrono>


namespace pml
{
    namespace nmos
    {
            class Constraints;

            template<typename T> class NMOS_EXPOSE connectionSender
            {
                public:
                    connectionSender(std::optional<bool> masterEnable, TransportParamsRTP::flagsTP allowed);
                    connectionSender(const connectionSender& conReq);
                    connectionSender& operator=(const connectionSender& other);

                    connectionSender(const Json::Value& jsResponse);

                    Json::Value GetJson() const;

                    const std::vector<TransportParamsRTPSender>& GetTransportParams() const { return m_vTransportParams;}

                    bool Patch(const Json::Value& jsData);

                    void Actualize(const std::vector<std::pair<std::string, std::string>>& vSourceDestIp);

                    std::optional<std::string> GetReceiverId() const;

                    void SetDestinationDetails(const std::vector<std::pair<std::string, unsigned short>>& vDestinations);

                    void Uninitialise();

                    static bool CheckJson(const Json::Value& jsData);
                    std::optional<bool> GetMasterEnable() const;
                    void MasterEnable(bool bEnable);

                    T& GetActivation() { return m_activation; }
                    const T& GetConstActivation() const { return m_activation; }

                    void EnableTransport(size_t nTP, bool bEnable);


                protected:
                    const Json::Value& GetJsonToCopy() const { return m_json; }
                    void SetTPAllowed(int flagsTransport);


                    Json::Value m_json;
                    T m_activation;
                    std::vector<TransportParamsRTPSender> m_vTransportParams;


            };

            //Connection API
            template<typename T> class NMOS_EXPOSE connectionReceiver
            {
                public:
                    connectionReceiver(std::optional<bool> masterEnable, TransportParamsRTP::flagsTP allowed);
                    connectionReceiver(const connectionReceiver& conReq);

                    connectionReceiver(int flagProperties);
                    connectionReceiver& operator=(const connectionReceiver& other);
                    connectionReceiver(const Json::Value& jsResponse);


                    std::optional<bool> GetMasterEnable() const;
                    void MasterEnable(bool bEnable);

                    T& GetActivation() { return m_activation; }
                    const T& GetConstActivation() const { return m_activation; }

                    Json::Value GetJson() const;
                    const std::vector<TransportParamsRTPReceiver>& GetTransportParams() const { return m_vTransportParams;}

                    void Actualize(const std::string& sInterfaceIp);


                    bool Patch(const Json::Value& jsData);

                    std::optional<std::string> GetSenderId() const;
                    std::optional<std::string> GetTransportFileType() const;
                    std::optional<std::string> GetTransportFileData() const;

                    void SetSenderId(const std::optional<std::string>& id);
                    void SetTransportFile(const std::optional<std::string>& type, const std::optional<std::string>& data);

                    void Uninitialise();

                    static bool CheckJson(const Json::Value& jsData);
                    void EnableTransport(size_t nTP, bool bEnable);

                protected:
                    const Json::Value& GetJsonToCopy() const { return m_json; }
                    void SetTPAllowed(int flagsTransport);

                    Json::Value m_json;
                    T m_activation;
                    std::vector<TransportParamsRTPReceiver> m_vTransportParams;

        };
    };
};
