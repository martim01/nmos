#pragma once
#include "nmosapiversion.h"
#include "transportparams.h"
#include "nmosdlldefine.h"
#include <chrono>


namespace pml
{
    namespace nmos
    {
            template<typename T> class NMOS_EXPOSE connection
            {
                public:
                    connection(std::experimental::optional<bool> masterEnable);
                    connection(const connection& conReq);
                    connection& operator=(const connection& other);

                    bool Patch(const Json::Value& jsData);
                    virtual Json::Value GetJson() const;

                    std::experimental::optional<bool> GetMasterEnable() const;
                    void MasterEnable(bool bEnable);

                    T& GetActivation() { return m_activation; }
                    const T& GetConstActivation() const { return m_activation; }



                protected:
                    virtual bool DoPatch(const Json::Value& jsData);

                    Json::Value m_json;
                    T m_activation;
            };

            template<typename T> class NMOS_EXPOSE connectionSender : public connection<T>
            {
                public:
                    connectionSender(std::experimental::optional<bool> masterEnable, size_t nTPLegs);
                    connectionSender(const connectionSender& conReq);
                    connectionSender& operator=(const connectionSender& other);
                    Json::Value GetJson() const override;

                    const std::vector<TransportParamsRTPSender>& GetTransportParams() const { return m_vTransportParams;}

                    void Actualize(const std::string& sSourceIp, const std::string& sDestinationIp);

                    void SetTPAllowed(int flagsTransport);
                    std::experimental::optional<std::string> GetReceiverId() const;

                    void SetDestinationDetails(const std::string& sIp, unsigned short nPort);

                    void Uninitialise();

                protected:
                    bool DoPatch(const Json::Value& jsData) override;

                    std::vector<TransportParamsRTPSender> m_vTransportParams;
            };

            //Connection API
            template<typename T> class NMOS_EXPOSE connectionReceiver : public connection<T>
            {
                public:
                    connectionReceiver(std::experimental::optional<bool> masterEnable, size_t nTPLegs);
                    connectionReceiver(const connectionReceiver& conReq);

                    connectionReceiver(int flagProperties);
                    connectionReceiver& operator=(const connectionReceiver& other);

                    Json::Value GetJson() const;
                    const std::vector<TransportParamsRTPReceiver>& GetTransportParams() const { return m_vTransportParams;}

                    void Actualize(const std::string& sInterfaceIp);
                    void SetTPAllowed(int flagsTransport);

                    std::experimental::optional<std::string> GetSenderId() const;
                    std::experimental::optional<std::string> GetTransportFileType() const;
                    std::experimental::optional<std::string> GetTransportFileData() const;

                    void SetSenderId(const std::experimental::optional<std::string>& id);
                    void SetTransportFile(const std::experimental::optional<std::string>& type, const std::experimental::optional<std::string>& data);

                    void Uninitialise();

                protected:
                    bool DoPatch(const Json::Value& jsData) override;
                    std::vector<TransportParamsRTPReceiver> m_vTransportParams;

                    //std::string sSenderId;
                    //std::string sTransportFileType;
                    //std::string sTransportFileData;

        };
    };
};
