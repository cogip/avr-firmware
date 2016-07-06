#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_ERROR    0
#define LOG_LEVEL_WARNING  1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_DEBUG    3

typedef void (*putchar_cb_t)(const char);
typedef int (*getchar_cb_t)(void);

#ifdef CONFIG_ENABLE_LOGGING
void print_log(int level, const char *function, const char *format, ...);
void log_init(putchar_cb_t, getchar_cb_t);
#define _print_log(level, ...) print_log(level, __FUNCTION__, __VA_ARGS__)
#else
#define log_init(cb) do {} while(0)
#define _print_log(level, ...) do {} while(0)
#endif

#define print_dbg(...) _print_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define print_info(...) _print_log(LOG_LEVEL_INFO, __VA_ARGS__)
#define print_warn(...) _print_log(LOG_LEVEL_WARNING, __VA_ARGS__)
#define print_err(...) _print_log(LOG_LEVEL_ERROR, __VA_ARGS__)

#endif /* LOG_H */
