#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define LOG_LEVEL_ERROR    0
#define LOG_LEVEL_WARNING  1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_DEBUG    3

#ifdef CONFIG_ENABLE_LOGGING
void print_log(int level, const char *function, const char *format, ...);
#define _print_log(level, ...) print_log(level, __FUNCTION__, __VA_ARGS__)
#else
#define _print_log(level, ...) do {} while(0)
#endif

#define print_dbg(...) _print_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define print_info(...) _print_log(LOG_LEVEL_INFO, __VA_ARGS__)
#define print_warn(...) _print_log(LOG_LEVEL_WARNING, __VA_ARGS__)
#define print_err(...) _print_log(LOG_LEVEL_ERROR, __VA_ARGS__)

typedef enum {
	COL_INT16,
	COL_DOUBLE,
	COL_END = -1,
} datalog_col_t;

#define COL_MAX 10

typedef struct {
	uint16_t line_cur;	/* current line index to write data to */
	uint16_t line_max;	/* number of line (i.e. vectors size) */

	uint8_t col_nb;
	struct {
		datalog_col_t	type;
		const char *	name;
	} columns[COL_MAX];

	void * datas[COL_MAX];
} datalog_t;

/*
 * Initialize a dynamic table of 'len' lines. Columns are created at runtime.
 * The variable argument list should finished by a COL_END entry.
 * For each column to register, at least 2 arguments are required:
 * 1. the column data type, and 2. the columns header string.
 *
 * Example to create a one column array of 100 lines:
 *   log_vect_init(&d, 100, COL_INT16, "Col1Header", COL_END);
 */
void log_vect_init(datalog_t *d, uint16_t len, ...);

/* Reset current logs data indexes to 0 */
void log_vect_reset(datalog_t *d);

/* Set value for a cell (ie. using column id) for the current line */
void log_vect_setvalue(datalog_t *d, uint8_t idx, void * value);

/* Move to next line for further data storage */
void log_vect_storeline(datalog_t *d);

/* Dump all table in CSV format */
void log_vect_dumpall(datalog_t *d);

#endif /* LOG_H */
