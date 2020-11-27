#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char *argv[]){

	if(argc==5){
		int num[4], i;

		for(i=0;i<4;i++){
			num[i] = atoi(argv[i+1]);
		}

		printf("%d %d\n",fibonacci(num[0]),max_of_four_int(num[0],num[1],num[2],num[3]));

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}


