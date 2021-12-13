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

response NmosServer::ConvertPostDataToJson(const postData& vData)
{
    response resp = JsonError(400, "No data sent");
    if(vData.size() == 1)
    {
        resp.nHttpCode = 200;
        resp.jsonData = ConvertToJson(vData[0].sData);
    }
    else if(vData.size() > 1)
    {
        resp.nHttpCode = 200;
        resp.jsonData.clear();
        for(size_t i = 0; i < vData.size(); i++)
        {
            if(vData[i].sName.empty() == false)
            {
                resp.jsonData[vData[i].sName] = vData[i].sData;
            }
        }
    }
    return resp;
}
