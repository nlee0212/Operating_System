#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "lib/user/syscall.h"
#include "devices/shutdown.h"
#include "devices/input.h"
void syscall_init (void);
int fibonacci(int);
int max_of_four_int(int,int,int,int);
void exit(int status);
void halt(void);
pid_t exec(const char* cmd_line);
int wait(pid_t pid);
int write(int fd, const void* buffer, unsigned size);
int read(int fd, void* buffer, unsigned size);
void check_user_vaddr(const void* addr);
#endif /* userprog/syscall.h */
