#include <iostream>
#include "registryapi.h"

using namespace std;

int main()
{
    RegistryApi::Get().Start(100, 8085, false);
    getchar();
    RegistryApi::Get().Stop();
    return 0;
}
