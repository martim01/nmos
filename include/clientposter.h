#pragma once
#include "eventposter.h"
#include "zcposter.h"

namespace pml
{
    namespace nmos
    {
        class ClientApiImpl;
        class ClientZCPoster : public pml::dnssd::ZCPoster
        {
            public:
                ClientZCPoster(ClientApiImpl* pImpl) : ZCPoster(), m_pImpl(pImpl){}
                virtual ~ClientZCPoster(){}
            protected:

                void InstanceResolved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance) override;
                void AllForNow(const std::string& sService) override;
                void Finished() override;
                void RegistrationNodeError() override;
                void InstanceRemoved(std::shared_ptr<pml::dnssd::dnsInstance> pInstance) override;

                ClientApiImpl* m_pImpl;

        };
    };
};
