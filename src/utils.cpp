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
#endif // __GNU__
using namespace std;


void SplitString(vector<string>& vSplit, string str, char cSplit)
{

    vSplit.clear();

    istringstream f(str);
    string s;
    while (getline(f, s, cSplit))
    {
        if(s.empty() == false || vSplit.empty())    //we don't want any empty parts apart from the base one
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
