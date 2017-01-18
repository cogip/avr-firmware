#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#if 0
/* #ifdef CONFIG_LOGGING_TIME */
#include <sys/time.h>

static void print_logtime(FILE *stream)
{
	struct timeval now;
	static struct timeval first = { 0, 0 };

	gettimeofday(&now, NULL);
	if (!first.tv_sec) {
		first.tv_sec = now.tv_sec;
		first.tv_usec = now.tv_usec;
	}
	if (now.tv_usec < first.tv_usec) {
		now.tv_sec--;
		now.tv_usec += 1000000;
	}
	now.tv_sec -= first.tv_sec;
	now.tv_usec -= first.tv_usec;

	fprintf(stream, "[%d.%06d]",
		(int)now.tv_sec, (int)now.tv_usec);
}
#else
#define print_logtime(stream) do { (void)(stream); } while(0)
#endif

static void print_log_v(int level, const char *function, const char *format,
		va_list args)
{
	FILE *stream = stdout;
	const char *prefix;
	const int dbg = CONFIG_DEFAULT_LOGLEVEL;

	if (level > dbg)
		return;

	switch (level) {
	case LOG_LEVEL_INFO:
		prefix = "<I>";
		break;
	case LOG_LEVEL_WARNING:
		prefix = "<W>";
		break;
	case LOG_LEVEL_ERROR:
		prefix = "<E>";
		break;
	case LOG_LEVEL_DEBUG:
		prefix = "<D>";
		break;
	default:
		prefix = "<U>";
		break;
	}

	fprintf(stream, "%s", prefix);
	print_logtime(stream);
	fprintf(stream, " %s(): ", function);

	vfprintf(stream, format, args);
}

void print_log(int level, const char *function, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	print_log_v(level, function, format, args);
	va_end (args);
}

void log_vect_init(datalog_t *d, const char *log_name, ...)
{
	va_list args;

	datalog_col_t t;

	d->line_cur = 0;
	if (log_name)
		strncpy (d->log_name, log_name, LOGNAME_MAX);

	d->col_nb = 0;

	va_start (args, log_name);

	for (t = va_arg(args, int);
	     t != COL_END;
	     d->col_nb++, t = va_arg(args, int))
	{
		/* 1st var arg is column type */
		d->columns[d->col_nb].type = t;

		/* 2nd var arg is the column name */
		d->columns[d->col_nb].name = va_arg(args, const char *);
	}

	va_end (args);

	return;
}

inline void log_vect_reset(datalog_t *d, const char *log_name)
{
	d->line_cur = 0;
	if (log_name)
		strncpy (d->log_name, log_name, LOGNAME_MAX);
}

void log_vect_setvalue(datalog_t *d, uint8_t idx, void * value)
{
	if (idx >= d->col_nb)
		return;

	switch (d->columns[idx].type) {
	case COL_INT16:
		d->datas[idx].as_int16 = *((int16_t *)value);
		break;
	case COL_DOUBLE:
		d->datas[idx].as_double = *((double *)value);
		break;
	case COL_END:
		break;
	}
}

static void _log_vect_print_header(datalog_t *d)
{
	uint8_t c;

	printf("\n<<<< %s.csv\n", d->log_name);
	printf("time,");
	for (c = 0; c < d->col_nb; c++) {
		printf("%s,", d->columns[c].name);
	}
	printf("\n");
}

void log_vect_display_line(datalog_t *d)
{
	uint8_t c;

	if (!d->line_cur)
		_log_vect_print_header(d);

	printf("%d,", d->line_cur);
	for (c = 0; c < d->col_nb; c++) {
		switch(d->columns[c].type) {
		case COL_INT16:
			printf("%d,", d->datas[c].as_int16);
			break;
		case COL_DOUBLE:
			printf("%+.2f,", d->datas[c].as_double);
			break;
		case COL_END:
			break;
		}
	}
	printf("\n");

	d->line_cur += 1;
}

void log_vect_display_last_line(datalog_t *d)
{
	log_vect_display_line(d);
	printf(">>>>\n");
}
