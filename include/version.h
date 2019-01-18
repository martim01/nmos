#pragma once
#include <string>
#include <sstream>

class ApiVersion
{
    public:
        ApiVersion(unsigned short nMaj, unsigned nMin) : m_nMajor(nMaj), m_nMinor(nMin){}
        ApiVersion(std::string sVersion)
        {
            if(sVersion.empty() == false)
            {
                if(sVersion[0] == 'v')
                {
                    sVersion = sVersion.substr(1);
                }
                try
                {
                    size_t nPos = sVersion.find(".");
                    if(nPos != std::string::npos)
                    {
                        m_nMajor = stoi(sVersion.substr(0,nPos-1));
                        m_nMinor = stoi(sVersion.substr(nPos+1));
                    }
                    else
                    {
                        m_nMajor = stoi(sVersion);
                        m_nMinor = 0;
                    }
                }
                catch(std::invalid_argument& ia)
                {
                    m_nMajor = 0;
                    m_nMinor = 0;
                }
            }
        }

        unsigned short GetMajor() const
        {   return m_nMajor;    }

        unsigned short GetMinor() const
        {   return m_nMinor;    }

        std::string GetVersionAsString() const
        {
            std::stringstream ss;
            ss << "V" << m_nMajor << "." << m_nMinor;
            return ss.str();
        }


    private:
        unsigned short m_nMajor;
        unsigned short m_nMinor;
};
