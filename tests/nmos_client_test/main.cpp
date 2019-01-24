#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "log.h"
#include "clientapi.h"
#include "self.h"

using namespace std;



int main()
{
    ClientApi::Get().Start();
    getchar();
    for(map<string, shared_ptr<Self> >::const_iterator itNode = ClientApi::Get().GetNodeBegin(); itNode != ClientApi::Get().GetNodeEnd(); ++itNode)
    {
        cout << itNode->second->GetId() << " :" << itNode->second->GetLabel() << endl;
    }
    ClientApi::Get().Stop();
}
