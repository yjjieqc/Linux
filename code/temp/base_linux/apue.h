/*included *after* all standard system headers*/
#ifndef _APUE_h
#define _APUE_h

#define _POSIX_C_SOURCE  200809L

#if defined(SOLARIS)
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 700
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/stat.h>
#include <stddef.h>
#include <signal.h>

#if defined(MACOS) || !defined(TIOCGWINSZ)
#include <sys/ioctl.h>
#endif

#define MAXLINE 4096

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP|S_IROTH)

#define DIR_MODE (FILE_MODE|S_IXUSR | S_IXGRP | S_IXOTH)

typedef void Sigfunc(int);
#if defined(SIG_IGN) && !defined(SIG_ERR)
#define SIG_ERR((Sigfunc*) -1)
#endif
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

char	*path_alloc(int*);
int		open_max(void);
void	clr_fl(int, int);
void	set_fl(int, int);
void	pr_exit(int);
void	pr_mask(const char *);
Sigfunc	*signal_intr(int, Sigfunc*);
void	daemonize(const char *);
void	sleep_us(unsigned int);
ssize_t readn(int, void*, size_t);
ssize_t writen(int, const void *, size_t);


#define read_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

pid_t	lock_test(int, int, off_t, int, off_t);


void	err_dump(const char *, ...);
void	err_msg(const char *, ...);
void	err_quit(const char *, ...);
void	err_ret(const char *, ...);
void	err_sys(const char *, ...);
void 	err_cont(int, const char*, ...);
void	err_exit(int, const char*, ...);

void	log_msg(const char *, ...);
void	log_open(const char *, ...);
void	log_quit(const char *, ...);
void	log_ret(const char *, ...);
void	log_sys(const char *, ...);
void	log_exit(const char *, ...);

void	TELL_WAIT(void);
void	TELL_PARENT(pid_t);
void	TELL_CHILD(pid_t);
void	WAIT_PARENT(void);
void	WAIT_CHILD(void);


static void err_doit(int, int, const char *, va_list);


void	err_ret(const char *fmt, ...){
	va_list	ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

void 	err_sys(const char *fmt, ...){
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

void    err_exit(int error, const char *fmt, ...){
        va_list ap;
        va_start(1, errno, fmt, ap);
        va_end(ap);
        exit(1);
}

void	err_dump(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort();
	exit();
}

void	err_msg(const char *fmt,...){
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

void 	err_quit(const char *fmt, ...){
	va_list	ap;
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap){
	char	buf[MAXLINE];

	vsnprintf(buf, MAXLINE-1,fmt, ap);
	if(errnoflag)
	snprintf(buf+strlen(buf), MAXLINE - strlen(buf)-1, ": %s", strerror(error));
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);
}

#endif