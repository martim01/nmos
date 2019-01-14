#include <iostream>
#include "registryapi.h"
#include "registrymemory.h"

using namespace std;

int main()
{
    RegistryApi::Get().Start(make_shared<RegistryMemory>(), 100, "eth0", 8085, false);
    getchar();
    RegistryApi::Get().Stop();
    return 0;
}
