#include <errno.h>
#include <stdarg.h>
#include "ourhdr"

static void err_doit(int, int, const char *, va_list);

void	err_ret(const char *fmt, ...){
	va_list	ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

void 	err_sys(const char *fmt, ..){
	va_list	ap;
	va_start(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

void    err_cont(int error, const char *fmt, ...){
        va_list ap;
        va_start(ap, fmt);
        err_doit(1, errno, fmt, ap);
        va_end(ap);
}

void    err_exit(int error, const char *fmt, ..){
        va_list ap;
        va_start(1, errno, fmt, ap);
        va_end(ap);
        exit(1);
}

void err_dump(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort();
	exit();
}

void err_msg(const char *fmt,...){
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}
