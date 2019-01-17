#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include "dlldefine.h"


class NMOS_EXPOSE LogOutput
{
    public:
        LogOutput(){}
        virtual ~LogOutput(){}
        virtual void Flush(int nLogLevel, const std::stringstream&  logStream);

};

class NMOS_EXPOSE Log
{
public:
    typedef std::ostream&  (*ManipFn)(std::ostream&);
    typedef std::ios_base& (*FlagsFn)(std::ios_base&);

    static const std::string STR_LEVEL[4];

    enum enumLevel{ LOG_DEBUG=0, LOG_INFO=1, LOG_WARN=2, LOG_ERROR=3 };

    static Log& Get(enumLevel eLevel=LOG_INFO);

    void SetOutput(LogOutput* pLogout)
    {
        if(m_pOutput)
        {
            delete m_pOutput;
        }
        m_pOutput = pLogout;
    }

    template<class T>  // int, double, strings, etc
    Log& operator<<(const T& output)
    {
        m_stream << output;
        return *this;
    }

    Log& operator<<(ManipFn manip) /// endl, flush, setw, setfill, etc.
    {
        manip(m_stream);

        if (manip == static_cast<ManipFn>(std::flush)
         || manip == static_cast<ManipFn>(std::endl ) )
            this->flush();

        return *this;
    }

    Log& operator<<(FlagsFn manip) /// setiosflags, resetiosflags
    {
        manip(m_stream);
        return *this;
    }

    Log& operator()(enumLevel e)
    {
        m_logLevel = e;
        return *this;
    }

    void SetLevel(enumLevel e)
    {
        m_logLevel = e;
    }

    void flush()
    {
    /*
      m_stream.str() has your full message here.
      Good place to prepend time, log-level.
      Send to console, file, socket, or whatever you like here.
    */
        m_pOutput->Flush(m_logLevel, m_stream);
        m_stream.str(std::string());
        m_stream.clear();
    }

private:
    Log() : m_logLevel(LOG_INFO) , m_pOutput(new LogOutput()){}
    ~Log()
    {
        if(m_pOutput)
        {
            delete m_pOutput;
        }
    }

    std::stringstream  m_stream;
    int                m_logLevel;
    LogOutput* m_pOutput;


};

