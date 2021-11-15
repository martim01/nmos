#include "zcposter.h"

namespace pml
{
    namespace nmos
    {
        class NodeApiPrivate;
        class NodeZCPoster : public ZCPoster
        {
            public:
                NodeZCPoster(NodeApiPrivate& api) : m_api(api){}

            protected:

                void InstanceResolved(std::shared_ptr<dnsInstance> pInstance) override;
                void AllForNow(const std::string& sService) override;
                void Finished() override;
                void RegistrationNodeError() override;
                void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance) override;

            private:
                NodeApiPrivate& m_api;
        };
    };
};
