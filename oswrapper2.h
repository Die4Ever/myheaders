//#define VISTA
#pragma once

#ifndef WIN32
#define LINUX
#endif

#ifdef _DEBUG
#define BDEBUG 1
//#define DEBUGLOG
#else
#define BDEBUG 0
#endif

#include <climits>

const int WIN_CTRL = 131072;
const int WIN_SHIFT = 65536;
const int WIN_ALT = 262144;

#ifdef WINRWLOCK
class RWLock
{
public:
	SRWLOCK lock;

	RWLock()
	{
		InitializeSRWLock(&lock);
	}

	int ReadLock()
	{
		AcquireSRWLockShared(&lock);
		return 1;
	}

	int ReadLock(int tries)
	{
		AcquireSRWLockShared(&lock);
		return 1;
	}

	int WriteLock()
	{
		AcquireSRWLockExclusive(&lock);
		return 1;
	}

	int WriteLock(int tries)
	{
		AcquireSRWLockExclusive(&lock);
		return 1;
	}

	void ReadUnlock()
	{
		ReleaseSRWLockShared(&lock);
	}

	void WriteUnlock()
	{
		ReleaseSRWLockExclusive(&lock);
	}
};
#endif
//#else
#if defined WIN32
#define popen _wpopen
#define pclose _pclose
#define _WINSOCKAPI_
#include <float.h>
#include <windows.h>
#define stricmp _stricmp

template<class ARG>
class Thread
{
public:
	ARG Arg;
	int (*Function)(ARG);
	HANDLE hThread;

	static DWORD WINAPI RunThread(void *ti)
	{
		Thread *thread = (Thread*)ti;
		thread->Function(thread->Arg);
		delete thread;
		return 0;
	}

	int StartThread(int (*function)(ARG), ARG arg)
	{
		Function = function;
		Arg = arg;
		hThread = CreateThread(NULL,0,RunThread,(void*)this,0,0);
		return 1;
	}
};

#define RaySleep(a) Sleep((a)/1000)
/*__inline void RaySleep(int micros)
{
	Sleep(micros/1000);
}*/

class CriticalSection
{
public:
	CRITICAL_SECTION cs;

#ifdef _DEBUG
	volatile int locked;
	const char *lastfile;
	unsigned int lastline;
#endif

	CriticalSection()
	{
		InitializeCriticalSection(&cs);
#ifdef _DEBUG
		locked=0;
		lastfile="";
		lastline=0;
#endif
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

#ifdef _DEBUG
#define cslock() _lock(__FILE__, __LINE__)

	void _lock(const char *file, unsigned int line)
#else
	void cslock()
#endif
	{
#ifdef _DEBUG
		for(unsigned int i=0;i<100;i++)
		{
			if(_trylock(file,line)==0)
				return;
			Sleep(10);
		}
#ifdef _IOSTREAM_
		cerr << "locking from "<<file<<":"<<line<<" is waiting on "<<lastfile<<":"<<lastline<<"\n";
#endif

#endif
		EnterCriticalSection(&cs);
#ifdef _DEBUG
		locked=1;
		lastfile=file;
		lastline=line;
#endif
	}

	void csunlock()
	{
		LeaveCriticalSection(&cs);
#ifdef _DEBUG
		locked=0;
#endif
	}

#ifdef _DEBUG
#define trycslock() _trylock(__FILE__, __LINE__)

	int _trylock(const char *file, unsigned int line)
#else
	int trycslock()
#endif
	{
		if(TryEnterCriticalSection(&cs))
		{
#ifdef _DEBUG
			locked=1;
			lastfile=file;
			lastline=line;
#endif
			return 0;//flipped for unix compatability
		}
		return 1;//flipped for unix compatability
		//return TryEnterCriticalSection(&cslock);
	}

};
#ifndef WINRWLOCK
class RWLock
{
public:
	//CRITICAL_SECTION lock;
	volatile long Readers;
	volatile long Waiting;
	volatile long Writers;
	DWORD WritingThread;
	//CRITICAL_SECTION ReadersLock;

	RWLock()
	{
		WritingThread = 0;
		Readers = 0;
		Waiting = 0;
		Writers = 0;
		//InitializeCriticalSection(&lock);
		//InitializeCriticalSection(&ReadersLock);
	}

	~RWLock()
	{
		//DeleteCriticalSection(&lock);
		//DeleteCriticalSection(&ReadersLock);
	}

	int ReadLock()
	{
		while(1)
		{
			if(InterlockedCompareExchangeAcquire(&Waiting, 1, 0)==0)
			{
				while(1)
				{
					if(Writers == 0)
					{
						Readers++;
						//Waiting--;
						//if(Waiting<0)
							Waiting=0;
						return 1;
					}
					Sleep(1);
				}
			}
			Sleep(1);
		}
		return 0;//should never be hit
	}

	int ReadLock(int tries)
	{
		while(tries!=0)
		{
			if(InterlockedCompareExchangeAcquire(&Waiting, 1, 0)==0)
			{
				while(tries!=0)
				{
					/*if(InterlockedCompareExchangeAcquire(&Writers, 1, 0)==0)
					{
						InterlockedIncrement(&Readers);
						InterlockedDecrement(&Writers);
						InterlockedDecrement(&Waiting);
						return 1;
					}*/
					if(Writers == 0)
					{
						Readers++;
						//Waiting--;
						//if(Waiting<0)
							Waiting=0;
						return 1;
					}
					tries--;
					Sleep(1);
				}
			}
			else
				tries--;
			Sleep(1);
		}
		return 0;
	}

	int WriteLock()
	{
		if( WritingThread != 0 && WritingThread == GetCurrentThreadId() )
		{
			Writers++;
			return 1;
		}
		while(1)
		{
			if(InterlockedCompareExchangeAcquire(&Waiting, 1, 0)==0)
			{
				while(1)
				{
					if(Writers == 0)
					{
						Writers = 1;
						while(1)
						{
							if(Readers == 0)
							{
								//Waiting--;
								//if(Waiting<0)
									Waiting=0;
									WritingThread = GetCurrentThreadId();
								return 1;
							}
							Sleep(1);
						}
					}
					Sleep(1);
				}
			}
			Sleep(1);
		}
		return 0;//should never be hit
	}

	int WriteLock(int tries)
	{
		if( WritingThread != 0 && WritingThread == GetCurrentThreadId() )
		{
			Writers++;
			return 1;
		}
		while(tries!=0)
		{
			if(InterlockedCompareExchangeAcquire(&Waiting, 1, 0)==0)
			{
				while(tries!=0)
				{
					/*if(InterlockedCompareExchangeAcquire(&Writers, 1, 0)==0)
					{
						while(tries!=0)
						{
							if(InterlockedCompareExchangeAcquire(&Readers, 0, 0)==0)
							{
								InterlockedDecrement(&Waiting);
								return 1;
							}
							tries--;
							Sleep(0);
						}
					}*/
					if(Writers == 0)
					{
						Writers = 1;
						while(tries!=0)
						{
							if(Readers == 0)
							{
								//Waiting--;
								//if(Waiting<0)
									Waiting=0;
									WritingThread = GetCurrentThreadId();
								return 1;
							}
							tries--;
							Sleep(1);
						}
					}
					tries--;
					Sleep(1);
				}
			}
			else
				tries--;
			Sleep(1);
		}
		return 0;
	}

	void ReadUnlock()
	{
		/*volatile */long a = Readers;
		if(a==0)
			return;
		while(1)
		{
			a = Readers;
			if(a<=0)
			{
				if(InterlockedCompareExchangeAcquire(&Readers, 0, a)==a)
				{
					return;
				}
			}
			else if(InterlockedCompareExchangeAcquire(&Readers, a-1, a)==a)
			{
				return;
			}
			Sleep(1);
		}
		//if(Readers>0)
		//	InterlockedDecrement(&Readers);
	}

	void WriteUnlock()
	{
		//WritingThread = 0;
		/*volatile */long a = Writers;
		if(a==0)
			return;
		while(1)
		{
			a = Writers;
			if(a<=1)
			{
				if(InterlockedCompareExchangeAcquire(&Writers, 0, a)==a)
				{
					WritingThread = 0;
					return;
				}
			}
			else if(InterlockedCompareExchangeAcquire(&Writers, a-1, a)==a)
			{
				return;
			}
			Sleep(1);
		}
		//if(Writers>0)
		//	InterlockedDecrement(&Writers);
	}

};
#endif

typedef HANDLE FILE_HANDLE;

__inline void rWriteFileOL(FILE_HANDLE file, void *buffer, __int64 pos, unsigned int bytestowrite)
{
	unsigned long a;
	OVERLAPPED lp;
	memset(&lp, 0, sizeof(OVERLAPPED));
	LARGE_INTEGER li;
	li.QuadPart = pos;
	lp.Offset = li.LowPart;
	lp.OffsetHigh = li.HighPart;
	lp.hEvent = CreateEvent( NULL, 1, 0, NULL);

	if(WriteFile(file, buffer, bytestowrite, &a, &lp) == 0 && GetLastError() == ERROR_IO_PENDING)
		GetOverlappedResult(file, &lp, &a, 1);
}

__inline void rReadFileOL(FILE_HANDLE file, void *buffer, __int64 pos, unsigned int bytestoread)
{
	unsigned long a;
	OVERLAPPED lp;
	memset(&lp, 0, sizeof(OVERLAPPED));
	LARGE_INTEGER li;
	li.QuadPart = pos;
	lp.Offset = li.LowPart;
	lp.OffsetHigh = li.HighPart;
	lp.hEvent = CreateEvent( NULL, 1, 0, NULL);

	if(ReadFile(file, buffer, bytestoread, &a, &lp) == 0 && GetLastError() == ERROR_IO_PENDING)
		GetOverlappedResult(file, &lp, &a, 1);
}

__inline void rWriteFile(FILE_HANDLE file, void *buffer, __int64 pos, int method, unsigned int bytestowrite)
{
	unsigned long a;
	SetFilePointerEx(file, *(LARGE_INTEGER*)&pos, 0, method);
	WriteFile(file, buffer, bytestowrite, &a, 0);
}

__inline void rReadFile(FILE_HANDLE file, void *buffer, __int64 pos, int method, unsigned int bytestoread)
{
	unsigned long a;
	SetFilePointerEx(file, *(LARGE_INTEGER*)&pos, 0, method);
	ReadFile(file, buffer, bytestoread, &a, 0);
}

__inline void rOpenFile(FILE_HANDLE &file, char *fname, __int64 reserve=0)
{
	file = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_FLAG_NO_BUFFERING, 0);

	int f = GetLastError();
	//if(f>0)
	//	cout << "f == " << f << "\n";
	if(f == 80)
	{
		CloseHandle(file);
		file = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_FLAG_NO_BUFFERING, 0);
		f = GetLastError();
	}

	if(reserve != 0)
	{
		LARGE_INTEGER li;
		GetFileSizeEx(file, &li);

		if(li.QuadPart < reserve)
		{
			li.QuadPart = reserve;
			SetFilePointerEx(file, li, 0, 0);
			SetEndOfFile(file);
		}
		else if(li.QuadPart > reserve)
		{
			/*li.QuadPart = reserve;
			SetFilePointerEx(file, li, 0, 0);
			SetEndOfFile(file);*/
		}
	}
}

__inline void rCloseFile(FILE_HANDLE file)
{
	CloseHandle(file);
}

__inline unsigned int GetMilliCount()
{
  // Something like GetTickCount but portable
  // It rolls over every ~ 12.1 days (0x100000/24/60/60)
  // Use GetMilliSpan to correct for rollover
  unsigned int nCount = (unsigned int)GetTickCount();
  return nCount;
}

__inline unsigned int GetMilliSpan( unsigned int nTimeStart )
{
  unsigned int nSpan = GetMilliCount() - nTimeStart;
  return nSpan;
}

#elif defined LINUX
#define __int64 long int
#define __int32 int
#define __int16 short
#define __int8 char
#define byte unsigned char
#define DWORD unsigned int
#define UINT unsigned int
#if __WORDSIZE == 64
#define UINT_PTR unsigned long int
#else
#define UINT_PTR unsigned int
#endif

/*typedef long int __int64;
typedef int __int32;
typedef short __int16;
typedef char __int8;
typedef unsigned char byte;
typedef unsigned int DWORD;
typedef unsigned int UINT_PTR;*/

#include <fenv.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#define stricmp strcasecmp

__inline void DebugBreak()
{
}

template<class ARG>
class Thread
{
public:
	ARG Arg;
	int (*Function)(ARG);
	//HANDLE hThread;
	pthread_t hThread;

	static void *RunThread(void *ti)
	{
		Thread *thread = (Thread*)ti;
		thread->Function(thread->Arg);
		delete thread;
		return NULL;
	}

	int StartThread(int (*function)(ARG), ARG arg)
	{
		Function = function;
		Arg = arg;
		//pthread_create( &hThread, NULL, Function,Arg);
		pthread_create( &hThread, NULL, RunThread,(void*)this);
		//hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Function,Arg,0,0);

		pthread_detach(hThread);
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
		return 1;
	}
};

#define RaySleep(a) usleep((a))

class CriticalSection
{
public:
	pthread_mutex_t cs;
#ifdef _DEBUG
	int locked;
	const char *lastfile;
	unsigned int lastline;
#endif

	CriticalSection()
	{
		pthread_mutexattr_t    attr;
		pthread_mutexattr_init(&attr);
#ifdef _DEBUG
		lastfile="";
		lastline=0;
		locked=0;
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#else
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
#endif
		pthread_mutex_init(&cs, &attr);
		pthread_mutexattr_destroy(&attr);
	}

	~CriticalSection()
	{
		pthread_mutex_destroy(&cs);
	}
#ifdef _DEBUG
#define cslock() _lock(__FILE__, __LINE__)

	void _lock(const char *file, unsigned int line)
#else
	void cslock()
#endif
	{
		int r = 0;
		//cout << "Locking "<<this<<" from "<<file<<":"<<line<<"...\n";
#ifdef _DEBUG
		for(unsigned int i=0;i<1000;i++)
		{
			r = trycslock();
			if(r==0 || r==EINVAL)
			{
				locked=1;
				lastfile=file;
				lastline=line;
				return;
			}
			else if(r!=EBUSY)
			{
				cerr << "Locking error on acquiring lock from "<<lastfile<<":"<<lastline<<" at "<<file<<":"<<line<<"!\n";
				cerr.flush();
				int a=0;
				a = 1/a;
			}
			RaySleep(1000);
		}
		cerr << "Acquiring lock from "<<lastfile<<":"<<lastline<<" at "<<file<<":"<<line<<" is taking a while...\n";
		cerr.flush();
#endif
		r = pthread_mutex_lock(&cs);
		if(r != 0 && r!=EINVAL)
		{
#ifdef _DEBUG
			cerr << "Locking error on acquiring lock from "<<lastfile<<":"<<lastline<<" at "<<file<<":"<<line<<"!\n";
#else
			cerr << "Locking error!\n";
#endif
			cerr.flush();
			int a=0;
			a = 1/a;
		}
#ifdef _DEBUG
		locked=1;
		lastfile=file;
		lastline=line;
#endif
	}

	void csunlock()
	{
		int r = 0;
		//cout << "Unlocking "<<this<<"...\n";
		r = pthread_mutex_unlock(&cs);
		if(r != 0 && r != EINVAL)
		{
			cerr << "Unlocking error!\n";
			cerr.flush();
			int a=0;
			a = 1/a;
		}
#ifdef _DEBUG
		locked=0;
#endif
	}

	int trycslock()
	{
		return pthread_mutex_trylock(&cs);
	}

};

__inline unsigned int GetMilliCount()
{
  // Something like GetTickCount but portable
  // It rolls over every ~ 12.1 days (0x100000/24/60/60)
  // Use GetMilliSpan to correct for rollover
  timeb tb;
  ftime( &tb );
  unsigned int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
  return nCount;
}

__inline unsigned int GetMilliSpan( unsigned int nTimeStart )
{
  unsigned int nSpan = GetMilliCount() - nTimeStart;
  return nSpan;
}

class RWLock
{
public:
	pthread_rwlock_t rwlock;// = PTHREAD_RWLOCK_INITIALIZER;

	RWLock()
	{
		rwlock=PTHREAD_RWLOCK_INITIALIZER;
	}

	~RWLock()
	{
		pthread_rwlock_destroy(&rwlock);
	}

	int ReadLock()
	{
		pthread_rwlock_rdlock(&rwlock);
		return 1;
	}

	int ReadLock(int tries)
	{
		pthread_rwlock_rdlock(&rwlock);
		return 1;
	}

	int WriteLock()
	{
		pthread_rwlock_wrlock(&rwlock);
		return 1;
	}

	int WriteLock(int tries)
	{
		pthread_rwlock_wrlock(&rwlock);
		return 1;
	}

	void ReadUnlock()
	{
		pthread_rwlock_unlock(&rwlock);
	}

	void WriteUnlock()
	{
		pthread_rwlock_unlock(&rwlock);
	}
};

#endif
//#endif
