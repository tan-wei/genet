#ifndef PLUGKIT_LOGGER_H
#define PLUGKIT_LOGGER_H

#include "export.h"
#include <stdarg.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Context Context;

#ifdef PLUGKIT_ENABLE_LOGGING
#define Log_debug(ctx, ...) Log_debug_(ctx, __FILE__, __LINE__, __VA_ARGS__)
#define Log_warn(ctx, ...) Log_warn_(ctx, __FILE__, __LINE__, __VA_ARGS__)
#define Log_info(ctx, ...) Log_info_(ctx, __FILE__, __LINE__, __VA_ARGS__)
#define Log_error(ctx, ...) Log_error_(ctx, __FILE__, __LINE__, __VA_ARGS__)
#else
#define Log_debug(ctx, ...)
#define Log_warn(ctx, ...)
#define Log_info(ctx, ...)
#define Log_error(ctx, ...)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define PLUGKIT_LOGGER_ATTR __attribute__((format(printf, 4, 5)))
#else
#define PLUGKIT_LOGGER_ATTR
#endif

void PLUGKIT_EXPORT Log_debug_(Context *ctx, const char *file, int line,
                               const char *format, ...) PLUGKIT_LOGGER_ATTR;
void PLUGKIT_EXPORT Log_warn_(Context *ctx, const char *file, int line,
                              const char *format, ...) PLUGKIT_LOGGER_ATTR;
void PLUGKIT_EXPORT Log_info_(Context *ctx, const char *file, int line,
                              const char *format, ...) PLUGKIT_LOGGER_ATTR;
void PLUGKIT_EXPORT Log_error_(Context *ctx, const char *file, int line,
                               const char *format, ...) PLUGKIT_LOGGER_ATTR;

#undef PLUGKIT_LOGGER_ATTR

PLUGKIT_NAMESPACE_END

#endif
