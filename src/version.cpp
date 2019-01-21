#include "version.h"
#include "log.h"

ApiVersion::ApiVersion(std::string sVersion)
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
                m_nMajor = stoi(sVersion.substr(0,nPos));
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

unsigned short ApiVersion::GetMajor() const
{   return m_nMajor;    }

unsigned short ApiVersion::GetMinor() const
{   return m_nMinor;    }

std::string ApiVersion::GetVersionAsString() const
{
    std::stringstream ss;
    ss << "V" << m_nMajor << "." << m_nMinor;
    return ss.str();
}


bool ApiVersion::operator<(const ApiVersion& version) const
{
    return ((m_nMajor < version.GetMajor() || (m_nMajor==version.GetMajor() && m_nMinor < version.GetMinor())));
}

bool ApiVersion::operator<=(const ApiVersion& version) const
{
    return (*this < version || *this == version);
}

bool ApiVersion::operator==(const ApiVersion& version) const
{
    return (m_nMajor == version.GetMajor() && m_nMinor == version.GetMinor());
}

bool ApiVersion::operator>=(const ApiVersion& version) const
{
    return (*this > version || *this == version);
}

bool ApiVersion::operator>(const ApiVersion& version) const
{
    return ((m_nMajor > version.GetMajor() || (m_nMajor==version.GetMajor() && m_nMinor > version.GetMinor())));
}

bool ApiVersion::operator!=(const ApiVersion& version) const
{
    return !(*this==version);
}
