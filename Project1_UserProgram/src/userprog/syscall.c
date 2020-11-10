#include "userprog/syscall.h"
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

/* Wrriten according to Pintos Manual 3.3.4 */
static void syscall_handler (struct intr_frame *);
/*
void exit(int status);
void halt(void);
pid_t exec(const char* cmd_line);
int wait(pid_t pid);
int write(int fd, const void* buffer, unsigned size);
int read(int fd, void* buffer, unsigned size);
void check_user_vaddr(const void* addr);
*/
void check_user_vaddr(const void* addr){
	if(is_kernel_vaddr(addr)) exit(-1);
}

int read(int fd, void* buffer, unsigned size){
	unsigned int i;
	if(fd==0){
		for(i=0;i<size;i++){
			((char*)buffer)[i] = (char)input_getc();
			if(((char*)buffer)[i]=='\0') break;
		}
		return i;
	}
	return size;
}

int write (int fd, const void* buffer, unsigned size){
	if(fd==1){
		putbuf((char*)buffer, size);
		return size;
	}
	return -1;
}

/* wait for child process until it finishes its work */
int wait(pid_t pid){
	return process_wait(pid);
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments,
   and returns the new process's program id (pid). Must return pid-1, which otherwise
   should not be a valid pid, if the program cannont load or run for any reason.
   Thus, the parent process cannot return from the exec until it knows whether the
   child process successfully loaded its exewcutable.
   You must use appropriate synchronization to ensure this.
*/
pid_t exec(const char* cmd_line){
	return process_execute(cmd_line);
}

/* Terminates Pintos by calling shutdown_power_off() (declared in 'devices/shutdown.h').
   This should be seldom used, because you lose some information about possible deadlock situations, etc.
*/
void halt(void){
	shutdown_power_off();
}

/* Terminates the current user program, returning status to the kernel.
   If the process's parent waits for it, this is the status that will be returned.
   Conventionally, a status of 0 indicates success and nonzero values indicate errors.
*/
void exit(int status){
	printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->exit_status = status;
	thread_exit();
}


void
syscall_init (void) 
{
	//printf("syscall enter????\n");
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf("syscall: %d\n",*(uint32_t *)(f->esp));
  //hex_dump(f->esp,f->esp,100,1);
  int i;

  switch (*(uint32_t *)(f->esp)) {
	case SYS_HALT:
	  halt();
	  break;
	case SYS_EXIT:
	  check_user_vaddr(f->esp + 4);
	  exit(*(uint32_t *)(f->esp + 4));
	  break;
	case SYS_EXEC:
	  check_user_vaddr(f->esp + 4);
	  f->eax = exec((const char*)*(uint32_t *)(f->esp + 4));
	  break;
	case SYS_WAIT:
	  check_user_vaddr(f->esp + 4);
	  f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
	  break;
	case SYS_CREATE:
	  break;
	case SYS_REMOVE:
	  break;
	case SYS_OPEN:
	  break;
	case SYS_FILESIZE:
	  break;
	case SYS_READ:
	  for(i=0;i<3;i++)
		  check_user_vaddr(f->esp + (20+4*i));
	  f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
	  break;
	case SYS_WRITE:
	  for(i=0;i<3;i++)
		  check_user_vaddr(f->esp + (20+4*i));
	  f->eax = write((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
	  break;
	case SYS_SEEK:
	  break;
	case SYS_TELL:
	  break;
	case SYS_CLOSE:
	  break;
	case SYS_FIB:
	  check_user_vaddr(f->esp + 4);
	  f->eax = fibonacci((int)*(uint32_t *)(f->esp + 4));
	  break;
	case SYS_MAX:
	  for(i=0;i<4;i++)
		  check_user_vaddr(f->esp + (28+4*i));
	  f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 28),(int)*(uint32_t *)(f->esp + 32),(int)*(uint32_t *)(f->esp + 36),(int)*(uint32_t *)(f->esp + 40));
	  break;
  }

  //printf ("system call!\n");
  //thread_exit ();
}

int fibonacci(int n){
	int i, now, before, fibo=0;
	if(n==0) return 0;
	if(n==1) return 1;

	now = 1;
	before = 0;

	for(i=2;i<=n;i++){
		fibo = now + before;
		before = now;
		now = fibo;
	}

	return fibo;
}

int max_of_four_int(int a, int b, int c, int d){
	if(a>=b){
		if(c>=d){
			if(a>=c) return a;
			return c;
		}
		if(a>=d) return a;
		return d;
	}

	if(c>=d){
		if(b>=c) return b;
		return c;
	}
	if(b>=d) return b;
	return d;
}
