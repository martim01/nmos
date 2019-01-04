#include "utils.h"
#include <istream>
#include <sstream>
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
