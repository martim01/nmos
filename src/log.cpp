#include "log.h"
#include <iostream>
#include "utils.h"

const std::string Log::STR_LEVEL[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};

Log& Log::Get(enumLevel eLevel)
{
    static Log lg;
    lg.SetLevel(eLevel);
    return lg;
}


void LogOutput::Flush(int nLogLevel, const std::stringstream&  logStream)
{
    //if(nLogLevel != Log::LOG_DEBUG)
    std::cout << GetCurrentTaiTime(true) << "\t" << Log::STR_LEVEL[nLogLevel] << "\t" << logStream.str();
}

