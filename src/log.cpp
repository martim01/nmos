#include "log.h"
#include <iostream>

const std::string Log::STR_LEVEL[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};

Log& Log::Get(enumLevel eLevel)
{
    static Log lg;
    lg.SetLevel(eLevel);
    return lg;
}


void LogOutput::Flush(int nLogLevel, const std::stringstream&  logStream)
{
    std::cout << Log::STR_LEVEL[nLogLevel] << "\t" << logStream.str();
}

