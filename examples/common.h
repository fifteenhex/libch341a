/*
 * common.h
 */

#ifndef EXAMPLES_COMMON_H_
#define EXAMPLES_COMMON_H_

#include <stdarg.h>

static int ch341a_log_cb(int level, const char *tag, const char *restrict format, ...)
{
	va_list(args);

	printf("%s:\t", tag);
	va_start(args, format);

	return vprintf(format, args);
}

#endif /* EXAMPLES_COMMON_H_ */
