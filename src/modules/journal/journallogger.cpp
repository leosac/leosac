/**
 * \file journallogger.cpp
 * \author Thibault Schueller <ryp.sqrt@gmail.com>
 * \brief event logger module class
 */

#include "journallogger.hpp"
#include "tools/version.hpp"

#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>

JournalLogger::JournalLogger(int logLevel)
:   _version(Version::buildVersionString(0, 1, 0)),
    _logLevel(logLevel)
{}

void JournalLogger::notify(const Event& event)
{
    std::stringstream   ss;
    std::time_t         now_c = std::chrono::system_clock::to_time_t(event.date);
    struct tm*          l = std::localtime(&now_c);

    if (event.logLevel > _logLevel)
        return;

    ss << l->tm_year + 1900 << '/';
    ss.fill('0');
    ss.width(2);
    ss << l->tm_mon + 1 << '/';
    ss.fill('0');
    ss.width(2);
    ss << l->tm_mday << ' ';
    ss.fill('0');
    ss.width(2);
    ss << l->tm_hour << 'h';
    ss.fill('0');
    ss.width(2);
    ss << l->tm_min << ' ';
    ss.fill('0');
    ss.width(2);
    ss << l->tm_sec << 's';

    std::clog << '[' << ss.str() << ']' << " " << Event::getLogLevelString(event.logLevel) << " ";
    if (!event.source.empty())
        std::clog << event.source << " said ";
    std::clog << '\"' << event.message << '\"' << std::endl;
}

IModule::Type JournalLogger::getType() const
{
    return (Logger);
}

const std::string& JournalLogger::getVersionString() const
{
    return (_version);
}