#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "userprog/exception.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "filesys/off_t.h"

struct file
{
  struct inode *inode;
  off_t pos;
  bool deny_write;
};

struct lock wrt_lock;
struct lock mutex;
int readcnt = 0;

static void syscall_handler(struct intr_frame *);
void check_user_vaddr(int*, int);

void check_user_vaddr(int* addr, int argc) {
    for (int i = 1; i <= argc; i++) {
        if (!is_user_vaddr(&addr[i]))
            exit(-1);
    }
}

void
syscall_init (void) 
{
  lock_init(&wrt_lock);
  lock_init(&mutex);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf("syscall: %d\n",*(uint32_t *)(f->esp));
  //hex_dump(f->esp,f->esp,100,1);
  int* sc_num = f->esp;
  thread_current()->esp = f->esp;

  if (!is_user_vaddr(f->esp))
      exit(-1);

  switch(*(uint32_t*)(f->esp)){
  case SYS_HALT:
      halt();
      break;
  case SYS_EXIT:
      check_user_vaddr(sc_num, 1);
      exit(sc_num[1]);
      break;
  case SYS_EXEC:
      check_user_vaddr(sc_num, 1);
      f->eax = (uint32_t)exec((const char*)sc_num[1]);
      break;
  case SYS_WAIT:
      check_user_vaddr(sc_num, 1);
        f->eax = (uint32_t)wait((pid_t)sc_num[1]);
      break;
  case SYS_CREATE:
      check_user_vaddr(sc_num, 2);
      f->eax = (uint32_t)create((const char*)sc_num[1], (unsigned)sc_num[2]);
      break;
  case SYS_REMOVE:
      check_user_vaddr(sc_num, 1);
      f->eax = (uint32_t)remove((const char*)sc_num[1]);
      break;
  case SYS_OPEN:
      check_user_vaddr(sc_num, 1);
      f->eax = (uint32_t)open((const char*)sc_num[1]);
      break;
  case SYS_FILESIZE:
      check_user_vaddr(sc_num, 1);
      f->eax = (uint32_t)filesize((int)sc_num[1]);
      break;
  case SYS_READ:
      check_user_vaddr(sc_num, 3);
      f->eax = (uint32_t)read((int)sc_num[1], (void *)sc_num[2], (unsigned)sc_num[3]);
      break;
  case SYS_WRITE:
      check_user_vaddr(sc_num, 3);
      f->eax = (uint32_t)write((int)sc_num[1], (const void *)sc_num[2], (unsigned)sc_num[3]);
      break;
  case SYS_SEEK:
      check_user_vaddr(sc_num, 2);
      seek((int)sc_num[1], (unsigned)sc_num[2]);
      break;
  case SYS_TELL:
      check_user_vaddr(sc_num, 1);
      f->eax=tell((int)sc_num[1]);
      break;
  case SYS_CLOSE:
      check_user_vaddr(sc_num, 1);
      close((int)sc_num[1]);
      break;
  case SYS_FIB:
      check_user_vaddr(sc_num, 1);
      f->eax = (uint32_t)fibonacci((int)sc_num[1]);
      break;
  case SYS_MAX:
      check_user_vaddr(sc_num, 4);
      f->eax = (uint32_t)max_of_four_int((int)sc_num[1], (int)sc_num[2], (int)sc_num[3], (int)sc_num[4]);
      break;
    default:
      exit(-1);
      break;
  }
}

void 
halt (void){
  shutdown_power_off();
}

void 
exit (int status){
  struct thread *cur = thread_current();
  printf("%s: exit(%d)\n", thread_name(), status);
  cur->exit_status = status;

  for (int i = 3; i < 128; i++){
    if (cur->fd[i] != NULL)
      close(i);
  }

  thread_exit();
}

pid_t 
exec (const char *cmd_line){
    return process_execute(cmd_line);
}

int 
wait (pid_t pid){
  return process_wait(pid);
}



bool 
create (const char *file, unsigned initial_size){
  if(file == NULL)
    exit(-1);
  
  return filesys_create(file, initial_size);
}

bool
remove (const char *file){
  if (file == NULL)
    exit(-1);
  
  return filesys_remove(file);
}

int 
open (const char *file){
  struct thread *cur = thread_current();
  struct file *f;
  int i, state = 0;

  if(file == NULL)
    exit(-1);
  

  lock_acquire(&wrt_lock);

  f = filesys_open(file);
  if (f){
    for (i = 3; i < 128; i++){
      if (cur->fd[i] == NULL){
        state = i;
        cur->fd[i] = f;
        if (!strcmp(file, thread_name()))
          file_deny_write(f);
        break;
      }
    }
  }
  else
    state = -1;

  lock_release(&wrt_lock);

  return state;
}

int 
filesize (int fd){
  return file_length(thread_current()->fd[fd]);
}

int
read(int fd, void* buffer, unsigned size) {
    int i;
    int ret = -1;

    if (buffer == NULL)
        exit(-1);
    if (!is_user_vaddr(buffer))
        exit(-1);
    lock_acquire(&mutex);
    readcnt++;
    if (readcnt == 1)
        lock_acquire(&wrt_lock);
    if (fd == 0) {
        for (i = 0; i < (int)size; i++) {
            ((uint8_t*)buffer)[i] = input_getc();
            if (((uint8_t*)buffer)[i] == 0)
                break;
        }
        readcnt--;
        if (readcnt == 0)
            lock_release(&wrt_lock);
        lock_release(&mutex);
        return i;
    }
    else if (fd >= 3 && thread_current()->fd[fd]) {
        ret = file_read(thread_current()->fd[fd], buffer, size);
    }
    readcnt--;
    if (readcnt == 0)
        lock_release(&wrt_lock);
    lock_release(&mutex);
    if (fd >= 3 && ret == -1)
        exit(-1);
    return ret;
}

int
write(int fd, const void* buffer, unsigned size) {
    int ret = -1;

    if (buffer == NULL)
        exit(-1);

    if (!is_user_vaddr(buffer))
        exit(-1);

    lock_acquire(&wrt_lock);

    if (fd == 1) {
        putbuf(buffer, size);
        ret = size;
    }

    else if (fd > 2 && thread_current()->fd[fd]) {
        if (thread_current()->fd[fd]->deny_write)
            file_deny_write(thread_current()->fd[fd]);
        ret = file_write(thread_current()->fd[fd], buffer, size);
    }

    lock_release(&wrt_lock);

    if (ret == -1)
        exit(-1);

    return ret;
}

void 
seek (int fd, unsigned position){
  file_seek(thread_current()->fd[fd], position);
}

unsigned 
tell (int fd){
  return file_tell(thread_current()->fd[fd]);
}

void 
close (int fd){
  struct file *f = thread_current()->fd[fd];

  if(f == NULL)
    exit(-1);
  thread_current()->fd[fd] = NULL;
  file_close(f);
}

int fibonacci(int n) {
    int i, now, before, fibo = 0;
    if (n == 0) return 0;
    if (n == 1) return 1;

    now = 1;
    before = 0;

    for (i = 2; i <= n; i++) {
        fibo = now + before;
        before = now;
        now = fibo;
    }

    return fibo;
}

int max_of_four_int(int a, int b, int c, int d) {
    if (a >= b) {
        if (c >= d) {
            if (a >= c) return a;
            return c;
        }
        if (a >= d) return a;
        return d;
    }

    if (c >= d) {
        if (b >= c) return b;
        return c;
    }
    if (b >= d) return b;
    return d;
}
