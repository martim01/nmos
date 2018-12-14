#pragma once
#include <iostream>
#include <sstream>
#include <string>


class Log
{
public:
    typedef std::ostream&  (*ManipFn)(std::ostream&);
    typedef std::ios_base& (*FlagsFn)(std::ios_base&);

    enum LogLevel
    {
        DEBUG=0,
        INFO=1,
        WARN=2,
        ERROR=3
    };

    static Log& Get(LogLevel eLevel=INFO);


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

    Log& operator()(LogLevel e)
    {
        m_logLevel = e;
        return *this;
    }

    void SetLevel(LogLevel e)
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

        std::cout << STR_LEVEL[m_logLevel] << "\t" << m_stream.str();
        m_stream.str( std::string() );
        m_stream.clear();
    }

private:
    Log() : m_logLevel(INFO) {}

    std::stringstream  m_stream;
    int                m_logLevel;

    static const std::string STR_LEVEL[4];
};

