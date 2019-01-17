#include "bonjourpublisher.h"
#include <sstream>
#include <thread>

#include "log.h"
using namespace std;

ServicePublisher::ServicePublisher(string sName, string sService, unsigned short nPort, string sHostname) :
    m_sName(sName),
    m_sService(sService),
    m_nPort(nPort),
    m_sHostname(sHostname)
{

}

ServicePublisher::~ServicePublisher()
{

}


//extern void HandleEvents(DNSServiceRef);

static void  RegisterCallBack(DNSServiceRef service,  DNSServiceFlags flags,  DNSServiceErrorType errorCode,  const char * name,  const char * type,  const char * domain, void * context)
{
    if (errorCode != kDNSServiceErr_NoError)
    {
        Log::Get(Log::LOG_ERROR) << "RegisterCallBack returned "<< errorCode << endl;
    }
    else
    {
        Log::Get(Log::LOG_INFO) << name << "." << type << "." << domain << "REGISTER" << endl;
    }
}


bool ServicePublisher::Start()
{

    m_sdRef = 0;
    Log::Get(Log::LOG_DEBUG) << "BojourPubliser: Start" << endl;


    DNSServiceErrorType error;
    DNSServiceRef serviceRef;

    const std::vector<unsigned char> vRecords(MakeTxtRecords());

    error = DNSServiceRegister(&m_sdRef,
                                0,                  // no flags
                                0,                  // all network interfaces
                                m_sName.c_str(),  // name
                                m_sService.c_str(),       // service type
                                "",                 // register in default domain(s)
                                NULL,               // use default host name
                                htons(m_nPort),        // port number
                                vRecords.size(),                  // length of TXT record
                                !vRecords.empty()? &vRecords[0] : NULL,               // no TXT record
                                NULL, // call back function
                                NULL);              // no context

    if (error == kDNSServiceErr_NoError)
    {
        Log::Get(Log::LOG_DEBUG) << "BojourPubliser: Started" << endl;
        m_mClientToFd[m_sdRef] = DNSServiceRefSockFD(m_sdRef);;

        thread th(RunSelect, this);
        th.detach();

        return true;
    }
    Log::Get(Log::LOG_ERROR) << "BojourPubliser: Failed to start: " << error << endl;
    return false;


}

void ServicePublisher::Stop()
{
    lock_guard<mutex> lock(m_mutex);
    if(m_sdRef)
    {
        DNSServiceRefDeallocate(m_sdRef);
    }
}

void ServicePublisher::Modify()
{
    lock_guard<mutex> lock(m_mutex);
    if(m_sdRef)
    {
        // try and find a record that matches
        const std::vector<unsigned char> txt_records = MakeTxtRecords();

        DNSServiceErrorType errorCode = DNSServiceUpdateRecord(m_sdRef, NULL, 0, (std::uint16_t)txt_records.size(), &txt_records[0], 0);

        if (errorCode != kDNSServiceErr_NoError)
        {
        }
    }
}

void ServicePublisher::AddTxt(string sKey, string sValue, bool bModify)
{
    m_mTxt[sKey] = sValue;
    if(bModify)
    {
        Modify();
    }
}

void ServicePublisher::RemoveTxt(string sKey, string sValue, bool bModify)
{
    m_mTxt.erase(sKey);
    if(bModify)
    {
        Modify();
    }
}

std::vector<unsigned char> ServicePublisher::MakeTxtRecords()
{
    vector<unsigned char> vText;



    for(map<string, string>::iterator itTxt = m_mTxt.begin(); itTxt != m_mTxt.end(); ++itTxt)
    {
        std::stringstream ss;
        ss << itTxt->first << "=" << itTxt->second;
        vText.push_back(ss.str().size());
        for(size_t i = 0; i < ss.str().size(); i++)
        {
            vText.push_back(ss.str()[i]);
        }

    }
    return vText;
}


void ServicePublisher::RunSelect(ServicePublisher* pPublisher)
{
    int count = 0;
	for ( ; ; )
	{
        if ( pPublisher->m_mClientToFd.size() == 0 )
		{
            //if(pPublisher->m_pPoster)
            //{
            //    Log::Get(Log::LOG_DEBUG) << "BonjourBrowser: Finished" << endl;
            //    pBrowser->m_pPoster->_Finished();
            //}
            break;
        }
		fd_set readfds;
		FD_ZERO(&readfds);
		int nfds = 0;
		for ( auto itClient = pPublisher->m_mClientToFd.cbegin() ; itClient != pPublisher->m_mClientToFd.cend() ; itClient++ )
		{
			FD_SET(itClient->second, &readfds);
			nfds = max((int)itClient->second, nfds);
		}


		Log::Get(Log::LOG_DEBUG) << "BonjourPublisher: Start select: fd =" << pPublisher->m_mClientToFd.size() << " nfds =" << nfds << endl;
		struct timeval tv = { 0, 1000 };

		//mDNSPosixGetFDSet(m, &nfds, &readfds, &tv);
		int result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

		if ( result > 0 )
		{
		    Log::Get(Log::LOG_DEBUG) << "BonjourPublisher: Select done"  << endl;
		    lock_guard<mutex> lock(pPublisher->m_mutex);
            //
            // While iterating through the loop, the callback functions might delete
            // the client pointed to by the current iterator, so I have to increment
            // it BEFORE calling DNSServiceProcessResult
            //
			for ( auto itClient =pPublisher-> m_mClientToFd.cbegin() ; itClient != pPublisher->m_mClientToFd.cend() ; )
			{
                auto jj = itClient++;
				if (FD_ISSET(jj->second, &readfds) )
				{
					DNSServiceErrorType err = DNSServiceProcessResult(jj->first);
                    if ( ++count > 10 )
					{
						break;
					}
				}
			}
		}
		else
		{
		    Log::Get(Log::LOG_DEBUG) << "Result = " << result << endl;
			break;
		}
        if ( count > 10 )
        {
			break;
		}
	}
}
