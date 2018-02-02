#pragma once

/*TODO
	-try to make a thread scalable allocator
		-will need to make OSWrapper macros for atomic functions
		-try using seperate pools to allocate from, locking only the chosen pool
			-randomly pick pools
				-2 threads could try the same pool at once
			-pick a pool by the hash of the thread id
				-less of a chance that 2 threads would try the same pool at once
				-pools could become imbalanced
				-producer/consumer would suffer(1 thread locking to allocate, the other thread locking the same pool to deallocate)
		-maybe try lock-free allocations and deallocations

	-bool IsAllocated(char *p) function?
*/
class NewAlloc
{
public:
	NewAlloc(unsigned int iPageSize, unsigned int iPages)
	{
	}

	char* AllocBytes(unsigned int bytes)
	{
		return new char[bytes];
	}

	void Dealloc(char *cptr)
	{
		delete[] cptr;
	}
};

class RayHeap
{
public:
	char *pEnd;
	char *pPages;
	unsigned int PageSize;
	unsigned int Pages;
	char *pi;
	unsigned int TFree;
	int iFree;

	RayHeap(unsigned int iPageSize, unsigned int iPages)
	{
		TFree=0;
		PageSize = iPageSize;
		iFree = Pages = iPages;

		pi = new char[Pages];
		memset(pi, 0, Pages);
		//for(int i=0;i<Pages)
		{
			pPages = new char[PageSize*Pages];
			pEnd = &pPages[PageSize*Pages];
		}
	}

	RayHeap()
	{
		TFree=0;
		PageSize = 16*1024;
		PageSize = 1024;
		iFree = Pages = 8192;

		pi = new char[Pages];
		memset(pi, 0, Pages);
		//for(int i=0;i<Pages)
		{
			pPages = new char[PageSize*Pages];
			pEnd = &pPages[PageSize*Pages];
		}
	}

	~RayHeap()
	{
		delete[] pi;
		//for(int i=0;i<Pages)
		{
			delete[] pPages;
		}
	}

	char* AllocBytes(unsigned int bytes)
	{
		if(bytes <= PageSize && iFree>0)
		{
			/*if(pi[TFree] == 0)
			{
				if(TFree+1<Pages)
					TFree++;
				else
					TFree--;

				pi[TFree]=1;
				iFree--;
				return &pPages[TFree*PageSize];
			}

			unsigned int i = strlen((char*)pi);
			if(i<Pages)
			{
				TFree = i + ( (i+1) <Pages);
				pi[i]=1;
				iFree--;
				return &pPages[i*PageSize];
			}*/

			unsigned int i = TFree;
			if(pi[i] != 0)
				i = (unsigned int)strlen((char*)pi);
			if(i<Pages)
			{
				TFree = i + ( (i+1) <Pages);
				pi[i]=1;
				iFree--;
				return &pPages[i*PageSize];
			}
		}
		//return NULL;
		return new char[bytes];
	}

	void Dealloc(char *cptr)
	{
		if(cptr>=pPages && cptr<pEnd)
		{
			unsigned int i = ((unsigned int)(cptr-pPages))/PageSize;
			pi[i]=0;
			if(i<TFree)
				TFree=i;
			iFree++;
			return;
		}
		delete[] cptr;
	}
};


class RayHeap2
{
public:
	char *pEnd;
	char *pPages;
	unsigned int PageSize;
	unsigned int Pages;
	char *pi;
	unsigned int TFree;
	unsigned int iFree;

	RayHeap2(unsigned int iPageSize, unsigned int iPages)
	{
		TFree=0;
		PageSize = iPageSize;
		iFree = Pages = iPages;

		pi = new char[Pages];
		memset(pi, 0, Pages);

		pPages = new char[PageSize*Pages];
		pEnd = &pPages[PageSize*Pages];
	}

	RayHeap2()
	{
		TFree=0;
		PageSize = 16*1024;
		PageSize = 1024;
		iFree = Pages = 8192;

		pi = new char[Pages];
		memset(pi, 0, Pages);

		pPages = new char[PageSize*Pages];
		pEnd = &pPages[PageSize*Pages];
	}

	~RayHeap2()
	{
		delete[] pi;
		delete[] pPages;
	}

	char* AllocBytes(unsigned int bytes)
	{
		//if(bytes <= PageSize && iFree>0)
		assert((bytes <= PageSize && iFree>0));
		{

			unsigned int i = TFree;
			if(pi[i] != 0)
				i = (unsigned int)strlen((char*)pi);
			//if(i<Pages)
			assert(i<Pages);
			{
				TFree = i + ( (i+1) <Pages);
				pi[i]=1;
				iFree--;
				return &pPages[i*PageSize];
			}
		}
		return NULL;
	}

	void Dealloc(char *cptr)
	{
		//if(cptr>=pPages && cptr<pEnd)
		assert(cptr>=pPages && cptr<pEnd);
		{
			unsigned int i = ((unsigned int)(cptr-pPages))/PageSize;
			pi[i]=0;
			if(i<TFree)
				TFree=i;
			iFree++;
			return;
		}
		//return 1;
		//delete[] cptr;
	}
};

/*char* RayHeap::AllocBytes(unsigned int bytes)
{
if(bytes < PageSize && iFree>0)
{
if(pi[TFree] == 0)
{
if(TFree+1<Pages)
TFree++;
else
TFree--;

pi[TFree]=1;
iFree--;
return &pPages[TFree*PageSize];
}

unsigned int i = strlen((char*)pi);
if(i<Pages)
{
TFree = i + ( (i+1) <Pages);
pi[i]=1;
iFree--;
return &pPages[i*PageSize];
}
}
return NULL;
//return new char[bytes];
}*/

//#define RayNew(heap, obj) new ((obj *)heap.AllocBytes( sizeof(obj))) obj()

//#define RayNew(heap, obj, args) new ((obj *)heap.AllocBytes( sizeof(obj))) obj args

//#define RayNewA(heap, obj, count) new ((obj*)heap.AllocBytes( sizeof(obj) * count)) obj[count]()
//#define RayDelete(heap, obj, ptr, args) ptr->~obj args, heap.Dealloc((char*)ptr), ptr=0


class RayHeapList
{
public:
	unsigned int PageSize;
	unsigned int Pages;
	unsigned int iHeaps;
	//vector<char> pi;
	//vector<RayHeap*> heaps;
	char	*pi;
	RayHeap2	**heaps;
	unsigned int TFree;
	unsigned int iFree;
	unsigned int AllocatedHeaps;

	RayHeapList(int iPageSize, int iPages)
	{
		iPageSize+=sizeof(void*);

		TFree = 0;
		PageSize = iPageSize;
		Pages = iPages;

		iHeaps = 4;

		pi = new char[iHeaps];
		memset(pi, 0, iHeaps);

		heaps = new RayHeap2*[iHeaps];
		memset(heaps, 0, iHeaps*sizeof(void*));

		heaps[0] = new RayHeap2(PageSize, Pages);
		iFree = 1;
		AllocatedHeaps = 0;
	}

	~RayHeapList()
	{
		for(unsigned int i=0;i<=AllocatedHeaps;i++)
		{
			delete heaps[i];
		}
		delete[] heaps;
		delete[] pi;
	}

	void upsizearrays()
	{
		unsigned int newHeaps = iHeaps*2;
		char *newpi = new char[newHeaps];
		RayHeap2 **newheaps = new RayHeap2*[newHeaps];

		memcpy(newpi, pi, iHeaps);
		memset(newpi+iHeaps, 0, newHeaps-iHeaps);

		memcpy(newheaps, heaps, iHeaps*sizeof(void*));
		memset(newheaps+iHeaps, 0, (newHeaps-iHeaps)*sizeof(void*));

		delete[] pi;
		delete[] heaps;
		pi = newpi;
		heaps = newheaps;
		iHeaps = newHeaps;
	}

	void downsizearrays()
	{
		unsigned int newHeaps = iHeaps/2;
		char *newpi = new char[newHeaps];
		RayHeap2 **newheaps = new RayHeap2*[newHeaps];

		memcpy(newpi, pi, newHeaps);

		memcpy(newheaps, heaps, newHeaps*sizeof(void*));

		delete[] pi;
		delete[] heaps;
		pi = newpi;
		heaps = newheaps;
		iHeaps = newHeaps;
	}

	char *AllocBytes(unsigned int bytes)
	{
		bytes += sizeof(void*);
		//if(bytes > PageSize)
		//	return NULL;
		assert(bytes <= PageSize);

		unsigned int i = TFree;
		if(pi[i]!=0)
			i = (unsigned int)strlen(pi);
		char *ret;
		//if(i<iHeaps)
		assert(i<iHeaps);
		{
			TFree = i;
			if(heaps[i] != NULL)
			{
				if(heaps[i]->iFree == 1)
				{
					TFree = i + ( (i+1) <Pages);
					pi[i] = 1;
					iFree--;
					if(iFree == 0 && pi[iHeaps-1] == 1)
					{
						//resize
						upsizearrays();
					}
				}
				ret = heaps[i]->AllocBytes(bytes);
				*((RayHeap2**)ret) = heaps[i];
				return ret+sizeof(void*);
			}
			else//allocate new heap
			{
				AllocatedHeaps = i;
				TFree = i;
				iFree++;
				heaps[i] = new RayHeap2(PageSize, Pages);
				ret = heaps[i]->AllocBytes(bytes);
				*((RayHeap2**)ret) = heaps[i];
				return ret+sizeof(void*);
			}
		}
		return NULL;
	}

	void Dealloc(char *cptr)
	{
		cptr -= sizeof(void*);
		RayHeap2* h = (*((RayHeap2**)cptr));

		if(h->iFree == 0)//find it and clear it's used flag
		{
			unsigned int i = 0;
			for(;i<iHeaps && h != heaps[i];i++);
			pi[i] = 0;
			iFree++;
		}
		h->Dealloc(cptr);
		if(h->iFree == h->Pages && iFree > 3)//find it and delete it
		{
			iFree--;
			unsigned int i = 0;
			for(;i<iHeaps && h != heaps[i];i++);
			delete heaps[i];
			heaps[i] = NULL;
			if(AllocatedHeaps != i)
			{
				heaps[i] = heaps[AllocatedHeaps];
				heaps[AllocatedHeaps] = NULL;
			}
			AllocatedHeaps--;

			if(AllocatedHeaps < iHeaps/4)
				downsizearrays();

			return;
		}
	}
};
