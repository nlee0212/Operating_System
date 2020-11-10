1.	개발 목표
user program이 실행되기까지의 일련의 과정을 이해하여 pintos 상에서 해당 user program을 올바르게 실행시키는 것이 이번 프로젝트의 목표이다.

2.	개발 범위 및 내용
 A.	개발 범위
  1.	Argument Passing
  원래 구현되어 있는 형식은 들어온 filename, 즉 command와 argument가 합쳐진 문자열이 바로 실행할 파일 이름으로 설정되어 있었다. 따라서 command 부분과 argument 부분을 분리하여 command의 이름으로 된 실행할 파일이 있는지 확인하는 코드를 구현하였다.
  
  in <userprog/process.c>
  makeup_stack(): 들어온 command와 argument들을 핀토스 매뉴얼 에서와 같은 방식으로 stack에 쌓았다.
  load(): 위에서 언급한 대로 command 실행 파일을 open한다. 문제 없이 open이 완료되었다면 filename을 위와 command와 argument 부분으로 분리하여 위 makeup_stack()함수를 호출하여 스택에 쌓았다. 
  
  2.	User Memory Access
  가장 중요한 점은 user memory space와 kernel memory space가 달랐다는 점이다. 따라서 user program이 kernel space를 침범하는 경우 프로그램을 종료 시켰다. check_userr_vaddr()를 새로 구현하였다.
  in <userprog/syscall.c>
  syscall_handler(): 각 시스템 콜 함수를 호출하기 전, 먼저 command와 argument들이 저장되어 있는 stack 내의 argument 주소들이 user space 내에 있는지를 다 확인하여 준다.
  in <userprog/exception.c>
  page_fault(): user변수나 접근한 주소가 kernel 주소라면 status -1을 넘겨주면서 exit()을 하게 하였다.
  
  3.	System Calls
  user program에서 사용하는 system call함수들이 정상적으로 동작할 수 있게끔 하였다.
  
  in <userprog/syscall.c>
  syscall_handler(): 위에서 메모리 영역이 정상적으로 user memory space안에 있는 것을 확인하였다면, system call type에 해당하는 system call 함수를 불러오는 함수를 호출하였다.
  exit(), wait() 등 system call 이름으로 된 함수들: system call 함수를 실행할 수 있는 조건이 있다면 확인해보고 system call 함수들을 call하여 해당 함수의 역할을 다 수행할 수 있게끔 한다.
  fibonacci(), max_of_four_int(): 추가적인 함수가 해당 역할을 수행하게끔 구현하였다.
  
  in <lib/user/syscall.c>
  syscall4(): argument가 4개인 system call을 받기 위한 함수
  fibonacci(), max_of_four_int(): uerprog에서 작성한 함수가 잘 call될 수 있도록 한다.

  in <lib/syscall-nr.h>
  각 system call함수에 대한 syscall number를 설정하는 곳. Fibonacci()와 max_of_four_int()를 위한 syscall number도 지정해 주게끔 하였다.
  in <threads/thread.h>
  struct thread: thread struct를 수정하여 필요한 추가적인 변수들 (exit_status, semaphore 등) 역시 추가로 정보를 저장할 수 있게끔 하였다. userprog내에서 필요하기 때문에 #ifdef USERPROG 내에서 추가적인 변수를 저장하였다.
  
  in <threads/thread.c>
  init_thread(): 부모 쓰레드는 자식 쓰레드의 존재, status 등을 알고 있어야 하기 때문에 부모와 자식을 연결해주는 내용을 구현하였다.

B.	개발 내용
-	Argument Passing
 
위와 같이 80x86 calling convention을 바탕으로 한 stack을 구현한다. esp의 시작점부터 위와 같은 방법으로 esp의 값을 감소해가며 적절한 메모리 공간에 적절한 data를 push 하였다. 프로그램에 입력된 명령어를 command 부분과 argument 부분들로 나누어 저장하게끔 하였다. 이는 strtok_r()을 사용하였다. 다 push 한 후, word-align을 해주었다. 그런 다음 0을 push한 후 각 argument의 주소값을 push한 다음argument count도 구하여 push한 후 return address를 저장하였다. 
-	User Memory Access
pintos상에서 invalid memory access는 user program이 kernel memory space를 침범하는 경우 또는 vice versa의 경우가 있다. 이러한 현상을 막기 위해서는 kernel이나 user가 각각 PHYS_BASE를 지나지 않는지 계속 확인해주어야 한다. user program내에서는 PHYS_BASE와 0xc00000000사이의 주소를 가지는지 확인해야 하는데, 이는 is_user_vaddr()로 확인할 수 있다. 반대의 경우는 is_kernel_vaddr()을 활용할 수 있다. 
-	System Calls
	시스템 콜의 필요성에 대한 간략한 설명
system call은 바로 kernel에 접근하여 원하는 일을 수행할 수 없는 user program이 kernel에 접근할 수 있는 역할을 한다. 보통 high-level 언어들은 시스템 운영에 핵심적인 부분을 차지하는 커널에 바로 접근이 불가능하기 때문에(위험함) system call을 통해 시스템에 접근할 수 있게 된다.
 

	이번 프로젝트에서 개발할 시스템 콜에 대한 간략한 설명
- wait(): child process가 끝날 때까지 parent가 terminate하지 않게 하기 위한 system call이다. child process의 exit status도 받아오는데, 만약 exit()을 통한 정상적인 중단이 아닌 kill()당했다면, -1을 return한다. parent가 wait()하기도 전에 child가 exit()을 하는 것은 허용되지만, 이럴 때에 parent가 그래도 정상적으로 exit status를 잘 받아오게끔 해야 한다.
- halt(): pintos를 shutdown_power_off()를 통해 중단한다. 
- exit(): 현재의 user process를 중단할 때 호출되며, kernel로 exit과 동시에 exit status를 return한다. 해당 exit status는 parent의 wait()에서 retrieve된다. 보통 0은 success, 0이 아닌 값은 nonsuccess로 간주된다.
- exec(): command로 들어온 명령어를 함께 들어온 argument들과 함께 수행한 후, new process의 pid를 return한다. 들어온 명령어가 수행 가능한 명령어인지 process_execute에서 확인하는데, 들어온 명령어의 tokenization도 여기 내부에서 수행한다. 만약 프로그램 파일이 실행되지 못하면 -1을 return한다. 

	유저 레벨에서 시스템 콜 API를 호출한 이후 커널을 거쳐 다시 유저 레벨로 돌아올 때까지 각 요소
1. open()을 사용하여 system call을 call한다.
2. syscall1/syscall2/syscall3/syscall4 중 하나가 호출 되며 (argument count에 따라 달라짐) system call number와 argument가 stack에 쌓이게 된다. 그러면서 ‘int $0x30’으로 interrupt를 발생시킨다.
3. interrupt를 위한 stack 역시 쌓은 후 interrupt handler를 호출한다.
4. interrupt handler는 system call handler를 호출한다.
5. system call handler에선 system call number에 따라 address가 valid한지 확인한 후 system call 함수를 호출한다. argument는 stack에서 esp의 주솟값을 기준으로 각각 access한다. 해당 system call 함수가 return 값이 있다면 eax에 집어넣는다.
