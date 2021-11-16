#pragma once
#include "eventposter.h"
#include "zcposter.h"

namespace pml
{
    namespace nmos
    {
        class ClientApiImpl;
        class ClientZCPoster : public ZCPoster
        {
            public:
                ClientZCPoster(ClientApiImpl* pImpl) : ZCPoster(), m_pImpl(pImpl){}
                virtual ~ClientZCPoster(){}
            protected:

                void InstanceResolved(std::shared_ptr<dnsInstance> pInstance) override;
                void AllForNow(const std::string& sService) override;
                void Finished() override;
                void RegistrationNodeError() override;
                void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance) override;

                ClientApiImpl* m_pImpl;

        };
    };
};
