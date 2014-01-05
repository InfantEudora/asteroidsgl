
#include "thread.h"

typedef void (*funcptr)();

Thread::Thread(){
	// We are not bound to a function.
	funct = (funcptr)NULL;	
}

void Thread::run(){ 
	/*
	int cnt = 0;
	int id = getthreadid();
	while(1){
		printf("THREAD: %i Count: %i \n",id,cnt);
		cnt++;
		sleep(1000);	
	}*/
	setPriority(THREAD_PRIORITY_LOWEST);

	if (funct != NULL){
		int id = getthreadid();
		printf("THREAD: %i Starting...\n",id);
		funct();
		printf("THREAD: %i Terminated...\n",id);
	}else
		printf("No Function\n");
}; 

void Thread::cleanup(){ 
}

void Thread::bind(void  *func){
	funct = (funcptr)func;	
}