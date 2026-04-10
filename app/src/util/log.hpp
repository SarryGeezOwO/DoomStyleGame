#ifndef GZ_LOG_HPP
#define GZ_LOG_HPP

namespace Geez
{
    // Use this for debug logging
    // And FORCE for release persist logs
    #ifdef GZ_BUILD_DEBUG
        #define GZ_LOG(type, ...)         Geez::Internal::Logger::Log(type, __VA_ARGS__)
        #define GZ_LOG_T(type, tabs, ...) Geez::Internal::Logger::Log(type, tabs, __VA_ARGS__)
        #define GZ_LOG_FORCE(type, ...)   Geez::Internal::Logger::Log(type, __VA_ARGS__)
        #define GZ_LOG_T_FORCE(type, tabs, ...) Geez::Internal::Logger::Log(type, tabs, __VA_ARGS__)
    #else
        #define GZ_LOG(type, ...)         ((void)0)
        #define GZ_LOG_T(type, tabs, ...) ((void)0)
        #define GZ_LOG_FORCE(type, ...)         Geez::Internal::Logger::Log(type, __VA_ARGS__)
        #define GZ_LOG_T_FORCE(type, tabs, ...) Geez::Internal::Logger::Log(type, tabs, __VA_ARGS__)
    #endif

    enum LogType
    {
        GZ_DEBUG    = 37,
        GZ_OK       = 34,
        GZ_SUCCESS  = 32,
        GZ_FAIL     = 33,
        GZ_FATAL    = 31,
        GZ_WARNING  = 35
    };

    namespace Internal
    {
        // logger that is called on release or not
        // use macro for debug only logs
        // You can still acess this class but that's on your
        // own will.
        struct Logger
        {
            // modify the global tab level
            static void set_tab_level(unsigned int tab_level);

            // Plus one tab level
            static void increment_tab_level(unsigned int amount);

            // Minus one tab level
            static void decrement_tab_level(unsigned int amount);

            static void enable_logging();
            static void disable_logging();

            // default logging
            static void Log(LogType type, const char* format, ...);

            // Ignores global tab levels and uses the provided tab level instead
            static void Log(LogType type, unsigned int tab_level, const char* format, ...);
        };
    }
}

#endif