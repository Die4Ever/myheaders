//#define VISTA

#ifdef VISTA
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

//#else
#elif defined WIN32
class RWLock
{
public:
	//CRITICAL_SECTION lock;
	volatile long Readers;
	volatile long Waiting;
	volatile long Writers;
	//CRITICAL_SECTION ReadersLock;

	RWLock()
	{
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
						Waiting--;
						if(Waiting<0)
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
						Waiting--;
						if(Waiting<0)
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
								Waiting--;
								if(Waiting<0)
									Waiting=0;
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
								Waiting--;
								if(Waiting<0)
									Waiting=0;
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
		volatile long a = Readers;
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
		volatile long a = Writers;
		if(a==0)
			return;
		while(1)
		{
			a = Writers;
			if(a<=0)
			{
				if(InterlockedCompareExchangeAcquire(&Writers, 0, a)==a)
				{
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
#elif defined LINUX
class RWLock
{
public:
	//CRITICAL_SECTION lock;
	volatile long Readers;
	volatile long Waiting;
	volatile long Writers;
	//CRITICAL_SECTION ReadersLock;

	RWLock()
	{
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
						Waiting--;
						if(Waiting<0)
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
						Waiting--;
						if(Waiting<0)
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
								Waiting--;
								if(Waiting<0)
									Waiting=0;
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
								Waiting--;
								if(Waiting<0)
									Waiting=0;
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
		volatile long a = Readers;
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
		volatile long a = Writers;
		if(a==0)
			return;
		while(1)
		{
			a = Writers;
			if(a<=0)
			{
				if(InterlockedCompareExchangeAcquire(&Writers, 0, a)==a)
				{
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
//#endif
