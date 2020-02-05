#include "server.h"
#include "nodeapi.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "self.h"
#include "device.h"
#include "source.h"
#include "flow.h"
#include "receiver.h"
#include "sender.h"
#include "log.h"
#include "eventposter.h"
#include "utils.h"
#include "curlregister.h"
#include "nmosserver.h"
#ifdef __GNU__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // __GNU__

using namespace std;



Server::Server(shared_ptr<EventPoster> pPoster, unsigned int nPort) : m_pPoster(pPoster), m_nPort(nPort)
{

}

Server::~Server()
{

}

bool Server::AddNmosControl(const string& sControl, shared_ptr<NmosServer> pServer)
{
    pServer->SetPoster(m_pPoster, m_nPort);
    return m_mServer.insert(make_pair(sControl, pServer)).second;
}


void Server::PrimeWait()
{
    lock_guard<mutex> lock(m_mutex);
    m_eOk = WAIT;
}

void Server::Wait()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    while(m_eOk == WAIT)
    {
        m_cvSync.wait(lk);
    }
}

void Server::Signal(bool bOk, const std::string& sData)
{
    lock_guard<mutex> lock(m_mutex);
    Log::Get(Log::LOG_DEBUG) << "Server: " << m_nPort << " Signal= " << bOk << endl;
    if(bOk)
    {
        m_eOk = SUCCESS;
    }
    else
    {
        m_eOk = FAIL;
    }
    m_sSignalData = sData;
    m_cvSync.notify_one();
}

bool Server::IsOk()
{
    lock_guard<mutex> lock(m_mutex);
    return (m_eOk == SUCCESS);
}




int Server::GetJson(std::string sPath, string& sReturn, std::string& sContentType)
{
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);
    sContentType = "application/json";

    int nCode = 200;
    SplitString(m_vPath, sPath, '/');


    if(m_vPath.empty())
    {
        Json::FastWriter stw;
        Json::Value jsNode;
        jsNode.append("x-nmos/");
        sReturn = stw.write(jsNode);
    }
    else
    {
        if(m_vPath[0] == "x-nmos")
        {   //check on nmos
            if(m_vPath.size() == 1)
            {
                nCode = GetApis(sReturn);
            }
            else
            {
                map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.find(m_vPath[1]);
                if(itServer != m_mServer.end())
                {
                    itServer->second->SetPath(m_vPath);
                    nCode = itServer->second->GetJsonNmos(this, sReturn, sContentType);
                }
                else
                {
                    Json::FastWriter stw;
                    sReturn = stw.write(GetJsonError(404, "Page not found"));
                    nCode = 404;
                }
            }
        }
        else
        {
            Json::FastWriter stw;
            sReturn = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    return nCode;
}





int Server::PutJson(std::string sPath, const string& sJson, string& sResponse)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::FastWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() > 1 && m_vPath[0] == "x-nmos")
    {
        map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.find(m_vPath[1]);
        if(itServer != m_mServer.end())
        {
            itServer->second->SetPath(m_vPath);
            nCode = itServer->second->PutJsonNmos(this, sJson, sResponse);
        }
        else
        {
            Json::FastWriter stw;
            sResponse = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    else
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(404, "Page not found"));
    }
    return nCode;
}


int Server::PatchJson(std::string sPath, const string& sJson, string& sResponse)
{
    Log::Get(Log::LOG_DEBUG) << "PatchJson" << std::endl;

    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::FastWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() > 1 && m_vPath[0] == "x-nmos")
    {
        map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.find(m_vPath[1]);
        if(itServer != m_mServer.end())
        {
            itServer->second->SetPath(m_vPath);
            nCode = itServer->second->PatchJsonNmos(this, sJson, sResponse);
        }
        else
        {
            Json::FastWriter stw;
            sResponse = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    else
    {
        nCode = 405;
        sResponse = stw.write(GetJsonError(nCode, "Method not allowed here."));
    }
    return nCode;
}

int Server::PostJson(std::string sPath, const string& sJson, string& sResponse)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::FastWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() > 1 && m_vPath[0] == "x-nmos")
    {
        map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.find(m_vPath[1]);
        if(itServer != m_mServer.end())
        {
            itServer->second->SetPath(m_vPath);
            nCode = itServer->second->PostJsonNmos(this, sJson, sResponse);
        }
        else
        {
            Json::FastWriter stw;
            sResponse = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    else
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(404, "Page not found"));
    }
    return nCode;
}

int Server::DeleteJson(std::string sPath, const string& sJson, string& sResponse)
{
    //make sure path is correct
    transform(sPath.begin(), sPath.end(), sPath.begin(), ::tolower);

    Json::FastWriter stw;

    int nCode = 202;
    SplitString(m_vPath, sPath, '/');
    if(m_vPath.size() > 1 && m_vPath[0] == "x-nmos")
    {
        map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.find(m_vPath[1]);
        if(itServer != m_mServer.end())
        {
            itServer->second->SetPath(m_vPath);
            nCode = itServer->second->DeleteJsonNmos(this, sJson, sResponse);
        }
        else
        {
            Json::FastWriter stw;
            sResponse = stw.write(GetJsonError(404, "Page not found"));
            nCode = 404;
        }
    }
    else
    {
        nCode = 404;
        sResponse = stw.write(GetJsonError(404, "Page not found"));
    }
    return nCode;
}

const std::string& Server::GetSignalData()
{
    return m_sSignalData;
}


int Server::GetApis(std::string& sReturn)
{
    Json::Value jsNode;
    for(map<string, shared_ptr<NmosServer> >::iterator itServer = m_mServer.begin(); itServer != m_mServer.end(); ++itServer)
    {
        jsNode.append(itServer->first+"/");
    }
    Json::FastWriter stw;
    sReturn = stw.write(jsNode);
    return 200;
}


Json::Value Server::GetJsonError(unsigned long nCode, const std::string& sError)
{
    Json::Value jsError(Json::objectValue);
    jsError["code"] = (int)nCode;
    jsError["error"] = sError;
    jsError["debug"] = "null";
    return jsError;
}

