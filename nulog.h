#ifndef NULOG_H
#define NULOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NULOG_VERSION "1.0.0"

#define NULOG_RED     "\x1b[31m"
#define NULOG_GREEN   "\x1b[32m"
#define NULOG_YELLOW  "\x1b[33m"
#define NULOG_BLUE    "\x1b[34m"
#define NULOG_MAGENTA "\x1b[35m"
#define NULOG_CYAN    "\x1b[36m"
#define NULOG_RESET   "\x1b[0m"

typedef enum {
    NULOG_LEVEL_DEBUG,
    NULOG_LEVEL_INFO,
    NULOG_LEVEL_WARN,
    NULOG_LEVEL_ERROR,
    NULOG_LEVEL_FATAL
} NuLogLevel;

typedef struct {
    NuLogLevel min_level;
    int show_timestamp;
    int show_source;
    int colored_output;
    FILE* output_stream;
} NuLogConfig;

static NuLogConfig nulog_config = {
    .min_level = NULOG_LEVEL_DEBUG,
    .show_timestamp = 1,
    .show_source = 1,
    .colored_output = 1,
    .output_stream = NULL
};

static const char* nulog_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

static const char* nulog_level_colors[] = {
    NULOG_BLUE,
    NULOG_GREEN,
    NULOG_YELLOW,
    NULOG_RED,
    NULOG_MAGENTA
};

static void nulog_write(NuLogLevel level, const char* source, const char* format, va_list args) {
    if (level < nulog_config.min_level) return;

    char timestamp[32] = {0};

    if (nulog_config.show_timestamp) {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    }

    FILE* stream = nulog_config.output_stream ? nulog_config.output_stream : stdout;

    if (nulog_config.show_timestamp) {
        fprintf(stream, "%s ", timestamp);
    }

    if (nulog_config.colored_output) {
        fprintf(stream, "%s%-5s%s ",
                nulog_level_colors[level],
                nulog_level_strings[level],
                NULOG_RESET);
    } else {
        fprintf(stream, "%-5s ", nulog_level_strings[level]);
    }

    if (nulog_config.show_source) {
        fprintf(stream, "%s: ", source);
    }

    vfprintf(stream, format, args);
    fprintf(stream, "\n");
    fflush(stream);
}

static inline void nulog_init() {
    nulog_config.output_stream = stdout;
}

static inline void nulog_configure(NuLogConfig config) {
    nulog_config = config;
    if (nulog_config.output_stream == NULL) {
        nulog_config.output_stream = stdout;
    }

    if (!isatty(fileno(nulog_config.output_stream))) {
        char timestamp[32] = {0};
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);

        if (nulog_config.colored_output) {
            nulog_config.colored_output = 0;
            fprintf(stderr, "[%s] WARNING : Colored output disabled for non-terminal stream. Set 'colored_output' to 0 to disable this message.\n", timestamp);
        }
    }
}

static inline void nulog_debug(const char* source, const char* format, ...) {
    va_list args;
    va_start(args, format);
    nulog_write(NULOG_LEVEL_DEBUG, source, format, args);
    va_end(args);
}

static inline void nulog_info(const char* source, const char* format, ...) {
    va_list args;
    va_start(args, format);
    nulog_write(NULOG_LEVEL_INFO, source, format, args);
    va_end(args);
}

static inline void nulog_warn(const char* source, const char* format, ...) {
    va_list args;
    va_start(args, format);
    nulog_write(NULOG_LEVEL_WARN, source, format, args);
    va_end(args);
}

static inline void nulog_error(const char* source, const char* format, ...) {
    va_list args;
    va_start(args, format);
    nulog_write(NULOG_LEVEL_ERROR, source, format, args);
    va_end(args);
}

static inline void nulog_fatal(const char* source, const char* format, ...) {
    va_list args;
    va_start(args, format);
    nulog_write(NULOG_LEVEL_FATAL, source, format, args);
    va_end(args);
}

#define NULOG_SOURCE __FILE__ ":" STRINGIFY(__LINE__)
#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

#define DEBUG(fmt, ...) nulog_debug(NULOG_SOURCE, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)  nulog_info(NULOG_SOURCE, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)  nulog_warn(NULOG_SOURCE, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) nulog_error(NULOG_SOURCE, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) nulog_fatal(NULOG_SOURCE, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // NULOG_H