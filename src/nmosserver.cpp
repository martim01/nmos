#include "nmosserver.h"

int NmosServer::GetJsonNmos(MicroServer* pServer, std::string& sReturn, std::string& sContentType)
{
    Json::StyledWriter stw;
    sReturn = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::PutJsonNmos(MicroServer* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::StyledWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::PatchJsonNmos(MicroServer* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::StyledWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}


Json::Value NmosServer::GetJsonError(unsigned long nCode, std::string sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}

void NmosServer::SetPath(const std::vector<std::string>& vPath)
{
    m_vPath = vPath;
}


void NmosServer::SetPoster(std::shared_ptr<EventPoster> pPoster,unsigned short nPort)
{
    m_pPoster = pPoster;
    m_nPort = nPort;
}
