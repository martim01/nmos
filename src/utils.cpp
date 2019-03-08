#include "utils.h"
#include <istream>
#include <sstream>
#ifdef __GNU__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//#include <uuid/uuid.h>
#include <sys/time.h>
#endif // __GNU__
#include <chrono>
#include <sstream>

#include "guid.h"
#include <array>


using namespace std;


static uuid_t NameSpace_OID = { /* 6ba7b812-9dad-11d1-80b4-00c04fd430c8 */
       0x6ba7b812,
       0x9dad,
       0x11d1,
       0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
   };

void SplitString(vector<string>& vSplit, string str, char cSplit)
{
    vSplit.clear();
    istringstream f(str);
    string s;
    while (getline(f, s, cSplit))
    {
        if(s.empty() == false)
        {
            vSplit.push_back(s);
        }
    }
}


string GetIpAddress(const string& sInterface)
{
    #ifdef __GNU__
    int fd = socket(AF_INET, SOCK_DGRAM,0);
    ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy((char*)ifr.ifr_ifrn.ifrn_name, sInterface.c_str(), IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);


    return inet_ntoa((((sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    #else
    return "";
    #endif
}


std::string GetCurrentTaiTime(bool bIncludeNano)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> tp(std::chrono::high_resolution_clock::now());
    tp += LEAP_SECONDS;
    return ConvertTimeToString(tp, bIncludeNano);
}

std::string ConvertTimeToString(std::chrono::time_point<std::chrono::high_resolution_clock> tp, bool bIncludeNano)
{
    std::stringstream sstr;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
    sstr << seconds.count();
    if(bIncludeNano)
    {
        sstr << ":" << (std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count()%1000000000);
    }
    return sstr.str();
}

std::string CreateGuid(std::string sName)
{
    uuid_t guid;
    uuid_create_md5_from_name(&guid, NameSpace_OID, sName.c_str(), sName.length());


    std::array<char,40> output;
    snprintf(output.data(), output.size(), "%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x",
             guid.time_low, guid.time_mid, guid.time_hi_and_version, guid.clock_seq_hi_and_reserved, guid.clock_seq_low,
             guid.node[0], guid.node[1], guid.node[2], guid.node[3], guid.node[4], guid.node[5]);
    return std::string(output.data());
}


std::string CreateGuid()
{
    return CreateGuid(GetCurrentTaiTime(true));
}


bool CheckJson(const Json::Value& jsObject, std::initializer_list<std::string> lstAllowed)
{
    for(Json::Value::const_iterator itParam = jsObject.begin(); itParam != jsObject.end(); ++itParam)
    {
        std::initializer_list<std::string>::iterator itList = lstAllowed.begin();
        for(; itList != lstAllowed.end(); ++itList)
        {
            if((*itList) == itParam.key().asString())
                break;
        }
        if(itList == lstAllowed.end())   //found a non allowed thing
        {
            return false;
        }
    }
    return true;
}
