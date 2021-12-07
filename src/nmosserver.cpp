#include "nmosserver.h"
#include <algorithm>
#include "utils.h"

using namespace pml::nmos;

NmosServer::NmosServer(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster, NodeApiPrivate& api) :
    m_pServer(pServer),
    m_version(version),
    m_pPoster(pPoster),
    m_api(api)
{

}

NmosServer::~NmosServer()
{

}

response NmosServer::JsonError(int nCode, const std::string& sError, const std::string& sDebug)
{
    response resp;
    resp.nHttpCode =nCode;
    resp.jsonData["code"] = nCode;
    resp.jsonData["error"] = sError;
    resp.jsonData["debug"] = sDebug;
    return resp;
}


std::vector<std::string> NmosServer::SplitEndpoint(const endpoint& theEndpoint)
{
    return SplitString(theEndpoint.Get(), '/');
}
