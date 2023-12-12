#pragma once
#include <string>
#include <condition_variable>
#include "microhttpd.h"
#include <thread>
#include <vector>
#include "json/json.h"
#include "version.h"
#include <sstream>


class NmosServer;
class EventPoster;

class Server
{
    public:

        /** @brief Constructor - this is called in NodeApi::StartService
        *   @param pPoster a sheared_ptr to an object of a class derived from EventPoster.
        *   @param nPort the port to listed for connections on
        **/
        Server(std::shared_ptr<EventPoster> pPoster,unsigned int nPort=80);

        ///< @brief Destructor
        virtual ~Server();

        /** @brief Starts the web server

        **/
        virtual bool Init()=0;

        /** Add ann NMOS server
        *   @param sControl the first part of the url (after the x-nmos/)
        *   @param pServer the server
        **/
        bool AddNmosControl(const std::string& sControl, std::shared_ptr<NmosServer> pServer);

        /** @brief Stope the web server
        **/
        virtual void Stop()=0;


        /** @brief Gets the port number that the web server is using to listen for connections
        *   @return <i>unsigned short</i> the port number
        **/
        unsigned short GetPort() const
        {
            return m_nPort;
        }

        void Wait();
        void PrimeWait();
        bool IsOk();
        void Signal(bool bOk, const std::string& sData);
        const std::string& GetSignalData();


    protected:

        int GetJson(std::string sPath, std::string& sResponse, std::string& sContentType);
        int PutJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PatchJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int PostJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);
        int DeleteJson(std::string sPath, const std::string& sJson, std::string& sRepsonse);

        int GetApis(std::string& sReturn);

        Json::Value GetJsonError(unsigned long nCode = 404, const std::string& sError="Resource not found");




        std::shared_ptr<EventPoster> m_pPoster;

        unsigned short m_nPort;


        std::vector<std::string> m_vPath;

        std::mutex m_mutex;
        std::condition_variable m_cvSync;
        enum enumSignal{WAIT, FAIL, SUCCESS};

        enumSignal m_eOk;

        std::string m_sSignalData;

        std::map<std::string, std::shared_ptr<NmosServer> > m_mServer;

};


