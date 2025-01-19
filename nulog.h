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
#include <stdlib.h>

#define NULOG_VERSION "1.1.0"
#define NULOG_MAX_STREAMS 8

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
    FILE* stream;
    int colored_output;
} NuLogStream;

typedef struct {
    NuLogLevel min_level;
    int show_timestamp;
    int show_source;
    NuLogStream streams[NULOG_MAX_STREAMS];
    size_t stream_count;
} NuLogConfig;

static NuLogConfig nulog_config = {
    .min_level = NULOG_LEVEL_DEBUG,
    .show_timestamp = 1,
    .show_source = 1,
    .streams = {{NULL, 0}},
    .stream_count = 0
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

    char message[4096] = {0};
    va_list args_copy;
    va_copy(args_copy, args);
    vsnprintf(message, sizeof(message), format, args_copy);
    va_end(args_copy);

    for (size_t i = 0; i < nulog_config.stream_count; i++) {
        FILE* stream = nulog_config.streams[i].stream;
        int colored = nulog_config.streams[i].colored_output;

        if (nulog_config.show_timestamp) {
            fprintf(stream, "%s ", timestamp);
        }

        if (colored) {
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

        fprintf(stream, "%s\n", message);
        fflush(stream);
    }
}

static inline void nulog_init() {
    nulog_config.streams[0].stream = stdout;
    nulog_config.streams[0].colored_output = isatty(fileno(stdout));
    nulog_config.stream_count = 1;
}

static inline int nulog_add_stream(FILE* stream, int colored_output) {
    char timestamp[32] = {0};
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);

    if (nulog_config.stream_count >= NULOG_MAX_STREAMS) {
        fprintf(stderr, "[%s] ERROR : Maximum number of streams reached (%d)\n", timestamp, NULOG_MAX_STREAMS);
        return 0;
    }

    if (!stream) {
        fprintf(stderr, "[%s] ERROR : Cannot add NULL stream\n", timestamp);
        return 0;
    }

    for (size_t i = 0; i < nulog_config.stream_count; i++) {
        if (nulog_config.streams[i].stream == stream) {
            return 1;
        }
    }

    if (colored_output && !isatty(fileno(stream))) {
        colored_output = 0;
        fprintf(stderr, "[%s] WARNING : Colored output disabled for non-terminal stream.\n", timestamp);
    }

    nulog_config.streams[nulog_config.stream_count].stream = stream;
    nulog_config.streams[nulog_config.stream_count].colored_output = colored_output;
    nulog_config.stream_count++;
    return 1;
}

static inline void nulog_remove_stream(const FILE* stream) {
    for (size_t i = 0; i < nulog_config.stream_count; i++) {
        if (nulog_config.streams[i].stream == stream) {
            for (size_t j = i; j < nulog_config.stream_count - 1; j++) {
                nulog_config.streams[j] = nulog_config.streams[j + 1];
            }
            nulog_config.stream_count--;
            break;
        }
    }
}

static inline void nulog_configure(NuLogConfig config) {
    nulog_config.min_level = config.min_level;
    nulog_config.show_timestamp = config.show_timestamp;
    nulog_config.show_source = config.show_source;
    nulog_config.stream_count = 0;

    for (size_t i = 0; i < config.stream_count && i < NULOG_MAX_STREAMS; i++) {
        nulog_add_stream(config.streams[i].stream, config.streams[i].colored_output);
    }

    if (nulog_config.stream_count == 0) {
        nulog_init();
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
