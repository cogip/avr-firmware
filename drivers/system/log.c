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

void log_vect_init(datalog_t *d, uint16_t len, ...)
{
	va_list args;

	datalog_col_t t;

	d->line_cur = 0;
	d->line_max = len;

	d->col_nb = 0;

	va_start (args, len);

	for (t = va_arg(args, int);
	     t != COL_END;
	     d->col_nb++, t = va_arg(args, int))
	{
		uint16_t size;

		/* 1st var arg is column type */
		d->columns[d->col_nb].type = t;

		switch (t) {
		case COL_INT16:
			size = sizeof(int16_t) * len;
			break;
		case COL_DOUBLE:
			size = sizeof(double) * len;
			break;
		default:
		case COL_END:
			return;
		}

		/* allocate a vector regarding column type element size */
		d->datas[d->col_nb] = malloc(size);
		if (!d->datas[d->col_nb])
			goto alloc_err;
		memset(d->datas[d->col_nb], 0, size);

		/* 2nd var arg is the column name */
		d->columns[d->col_nb].name = va_arg(args, const char *);
	}

	va_end (args);

	return;

alloc_err:
	printf("Malloc failed!\n");
#if defined(__AVR__)
	printf("%p, %p, %d\n",
		__malloc_heap_end,
		__malloc_heap_start,
		__malloc_margin);
#endif

	for(;;);
}

inline void log_vect_reset(datalog_t *d)
{
	d->line_cur = 0;
}

void log_vect_setvalue(datalog_t *d, uint8_t idx, void * value)
{
	int16_t *i_datas;
	double  *d_datas;

	if (idx >= d->col_nb)
		return;
	if (d->line_cur >= d->line_max)
		return;

	switch (d->columns[idx].type) {
	case COL_INT16:
		i_datas = (int16_t *)d->datas[idx];
		i_datas[d->line_cur] = *((int16_t *)value);
		break;
	case COL_DOUBLE:
		d_datas = (double *)d->datas[idx];
		d_datas[d->line_cur] = *((double *)value);
		break;
	case COL_END:
		break;
	}
}

void log_vect_storeline(datalog_t *d)
{
	if (d->line_cur >= d->line_max)
		return;
	d->line_cur += 1;
//	d->line_cur %= d->line_max;
}

void log_vect_dumpall(datalog_t *d)
{
	uint8_t c;
	uint16_t l;

	int16_t *i_datas;
	double  *d_datas;

	printf("\ntime,");
	for (c = 0; c < d->col_nb; c++) {
		printf("%s,", d->columns[c].name);
	}
	printf("\n");

	for (l = 0; l < d->line_cur; l++) {
		printf("%d,", l);
		for (c = 0; c < d->col_nb; c++) {
			switch(d->columns[c].type) {
			case COL_INT16:
				i_datas = (int16_t *)d->datas[c];
				printf("%d,", i_datas[l]);
				break;
			case COL_DOUBLE:
				d_datas = (double *)d->datas[c];
				printf("%+.2f,", d_datas[l]);
				break;
			case COL_END:
				break;
			}
		}
		printf("\n");
	}
}
