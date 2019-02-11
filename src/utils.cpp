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
#include <uuid/uuid.h>
#include <sys/time.h>
#endif // __GNU__
using namespace std;


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


std::string CreateGuid()
{

#ifdef __GNUWIN32__
    UUID guid;
	CoCreateGuid(&guid);

    std::stringstream os;
//	os << std::lowercase;
    os.width(8);
    os << std::hex << guid.Data1 << '-';

    os.width(4);
    os << std::hex << guid.Data2 << '-';

    os.width(4);
    os << std::hex << guid.Data3 << '-';

    os.width(2);
    os << std::hex
        << static_cast<short>(guid.Data4[0])
        << static_cast<short>(guid.Data4[1])
        << '-'
        << static_cast<short>(guid.Data4[2])
        << static_cast<short>(guid.Data4[3])
        << static_cast<short>(guid.Data4[4])
        << static_cast<short>(guid.Data4[5])
        << static_cast<short>(guid.Data4[6])
        << static_cast<short>(guid.Data4[7]);
  //  os << std::nolowercase;
    return  os.str();
#endif // __WIN__

#ifdef __GNU__
    uuid_t guid;
    uuid_generate(guid);

    char uuid_str[37];      // ex. "1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
    uuid_unparse_lower(guid, uuid_str);
    return uuid_str;
#endif // __GNU__
}
