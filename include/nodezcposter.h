#include "zcposter.h"

class NodeZCPoster : public ZCPoster
{
    public:
        NodeZCPoster(){}

    protected:

        virtual void InstanceResolved(std::shared_ptr<dnsInstance> pInstance);
        virtual void AllForNow(const std::string& sService);
        virtual void Finished();
        virtual void RegistrationNodeError();
        virtual void InstanceRemoved(std::shared_ptr<dnsInstance> pInstance);

};

