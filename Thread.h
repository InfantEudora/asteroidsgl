#ifndef THREAD_H 
#define THREAD_H 
  
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <tchar.h>

  
namespace threading{ 
	class thread{ 
		static DWORD WINAPI ThreadFunc(LPVOID pv){ 
			try{ 
				(reinterpret_cast<thread *>(pv))->run(); 
				}
			catch(...){ 
				printf_s("Reinterpret failed.\n");
			} 
			return 0; 
		} 


	public:
		HANDLE hthread;
		typedef DWORD threadid;   
		thread(){} 
		virtual ~thread(){}

		static threadid getthreadid(){ 
			return ::GetCurrentThreadId(); 
		}


		



		static void sleep(long milliseconds=1){ 
			::Sleep(milliseconds); 
		} 

		threadid start(){ 
			threadid id; 
			hthread = ::CreateThread(NULL, 0, ThreadFunc, this, 0, &id); 
			setPriority(THREAD_PRIORITY_NORMAL);
			return id; 
		} 
  
		virtual void run()=0; 
		virtual void cleanup()=0; 

		void kill(){
			if (hthread){				
				::TerminateThread( hthread, 0 );
				::CloseHandle(hthread);
				cleanup();
				hthread = NULL;
			}
		}

		void setPriority(int tp) {
			if(hthread == NULL) {
				printf_s("Thread object is null\n");
			}else {
				if(SetThreadPriority(hthread,tp) == 0) {
					printf_s("Failed to set priority.\n");
				}
			}
		}
	};
}; 


class Mutex {
	private:
		// unsigned long* to the low-level mutex object
		unsigned long* m_hMutex;
		// name to identify the mutex
		char m_strName[16];
	public:
		Mutex();
		Mutex(const char* nm);
		void create(const char* nm);
		unsigned long* getMutexHandle();
		char* getName();
		void release();
		~Mutex();
};


class Thread : public threading::thread{ 
	public: 
		void (*funct)(void);
		
		Thread();
		virtual void bind(void  *func);
		virtual void run();
		virtual void cleanup(); 
}; 


#endif