#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

#define MAX_LOG_LEVEL LOG_LEVEL_DEBUG

void LogMessage(int level, const char *format, ...);

#if defined(DEBUG) || defined(_DEBUG)

#if (MAX_LOG_LEVEL >= LOG_LEVEL_ERROR)
#define LogError(format, ...)                                                  \
    LogMessage(LOG_LEVEL_ERROR, "[%s] " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define LogError(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_WARNING)
#define LogWarning(format, ...)                                                \
    LogMessage(LOG_LEVEL_WARNING, "[%s] " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define LogWarning(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_INFO)
#define LogInfo(format, ...)                                                   \
    LogMessage(LOG_LEVEL_INFO, "[%s] " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define LogInfo(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define LogDebug(format, ...)                                                  \
    LogMessage(LOG_LEVEL_DEBUG, "[%s] " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define LogDebug(format, ...)
#endif

#else

#if (MAX_LOG_LEVEL >= LOG_LEVEL_ERROR)
#define LogError(format, ...) LogMessage(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#else
#define LogError(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_WARNING)
#define LogWarning(format, ...)                                                \
    LogMessage(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#else
#define LogWarning(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_INFO)
#define LogInfo(format, ...) LogMessage(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#else
#define LogInfo(format, ...)
#endif
#if (MAX_LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define LogDebug(format, ...) LogMessage(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#else
#define LogDebug(format, ...)
#endif

#endif

#endif // LOG_H