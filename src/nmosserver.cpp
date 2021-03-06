#include "nmosserver.h"

int NmosServer::GetJsonNmos(Server* pServer, std::string& sReturn, std::string& sContentType)
{
    Json::FastWriter stw;
    sReturn = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::PutJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::FastWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::PatchJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::FastWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::PostJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::FastWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

int NmosServer::DeleteJsonNmos(Server* pServer, const std::string& sJson, std::string& sResponse)
{
    Json::FastWriter stw;
    sResponse = stw.write(GetJsonError(405, "Method not allowed here."));
    return 405;
}

Json::Value NmosServer::GetJsonError(unsigned long nCode, const std::string& sError)
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
