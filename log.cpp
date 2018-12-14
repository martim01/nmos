#include "log.h"
#include <iostream>

const std::string Log::STR_LEVEL[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};

Log& Log::Get(LogLevel eLevel)
{
    static Log lg;
    lg.SetLevel(eLevel);
    return lg;
}


