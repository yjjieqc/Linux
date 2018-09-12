/*included *after* all standard system headers*/
#ifndef _ourhdr_h
#define _ourhdr_h

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/stat.h>

#define MAXFILE 4096
#define _POSIX_C_SOURCE  200809L

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP|S_IROTH)

#define DIR_MODE (FILE_MODE|S_IXUSR | S_IXGRP | S_IXOTH)

typedef void Sigfunc(int);
#if defined(SIG_IGN) && !defined(SIG_ERR)
#define SIG_ERR((Sigfunc*) -1)
#endif
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

char *path_alloc(int*)
int open_max(void);
void clr_fl(int, int);
void	set_fl(int, int);
void	pr_exit(int);
void	pr_mask(const char *);
Sigfunc	*signal_intr(int, Sigfunc*);

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

