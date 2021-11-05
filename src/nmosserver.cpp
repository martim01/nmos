#include "nmosserver.h"
#include <algorithm>
#include "utils.h"

NmosServer::NmosServer(std::shared_ptr<RestGoose> pServer, const ApiVersion& version, std::shared_ptr<EventPoster> pPoster) :
    m_pServer(pServer),
    m_version(version),
    m_pPoster(pPoster)
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


std::vector<std::string> NmosServer::SplitUrl(const url& theUrl)
{
    return SplitString(theUrl.Get(), '/');
}
