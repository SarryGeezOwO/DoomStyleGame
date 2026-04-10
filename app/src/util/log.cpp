#include "log.hpp"
#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <sstream>

static unsigned int global_tab_Level = 0;
static bool logging_enabled = true;

static const char* GetLogTypeName(Geez::LogType type) {
    switch (type)
    {
    case Geez::GZ_DEBUG: return "DEBUG";
    case Geez::GZ_OK: return "OK";
    case Geez::GZ_SUCCESS: return "SUCCESS";
    case Geez::GZ_FAIL: return "FAIL";
    case Geez::GZ_FATAL: return "FATAL";
    case Geez::GZ_WARNING: return "WARNING";
    }
    return "";
}

static void log_internal(Geez::LogType type, unsigned int tab_level, const char* format, va_list args)
{
    if (!logging_enabled) return;

    std::stringstream str;
    for (unsigned int i = 0; i < tab_level; ++i)
        str << "+ ";

    str << "\033[" << type << "m"
        << "[" << GetLogTypeName(type) << "] "
        << format
        << "\033[0m\n";

    vprintf(str.str().c_str(), args);
}

void Geez::Internal::Logger::set_tab_level(unsigned int tab_level)
{
    global_tab_Level = tab_level;
}

void Geez::Internal::Logger::increment_tab_level(unsigned int amount)
{
    global_tab_Level += amount;
}

void Geez::Internal::Logger::decrement_tab_level(unsigned int amount)
{
    if (global_tab_Level - amount <= 0) {
        global_tab_Level = 0;
        return;
    }
    global_tab_Level -= amount;
}

void Geez::Internal::Logger::enable_logging()
{
    logging_enabled = true;
}

void Geez::Internal::Logger::disable_logging()
{
    logging_enabled = false;
}

void Geez::Internal::Logger::Log(LogType type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(type, global_tab_Level, format, args);
    va_end(args);
}

void Geez::Internal::Logger::Log(LogType type, unsigned int tab_level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(type, tab_level, format, args);
    va_end(args);
}