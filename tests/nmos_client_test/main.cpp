#include <iostream>
//#include "json/json.h"
#include <iostream>
#include <fstream>
#include "log.h"
#include "clientapi.h"

using namespace std;



int main()
{
    ClientApi::Get().Start();
    getchar();
    ClientApi::Get().Stop();
}
