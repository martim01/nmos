#pragma once
#include <string>
#include <sstream>

class ApiVersion
{
    public:
        ApiVersion(unsigned short nMaj, unsigned nMin) : m_nMajor(nMaj), m_nMinor(nMin){}
        ApiVersion(std::string sVersion);

        unsigned short GetMajor() const;
        unsigned short GetMinor() const;
        std::string GetVersionAsString() const;

        bool operator<(const ApiVersion& version) const;
        bool operator<=(const ApiVersion& version) const;
        bool operator==(const ApiVersion& version) const;
        bool operator>=(const ApiVersion& version) const;
        bool operator>(const ApiVersion& version) const;
        bool operator!=(const ApiVersion& version) const;

    private:
        unsigned short m_nMajor;
        unsigned short m_nMinor;
};
