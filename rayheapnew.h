#pragma once

template <UINT_PTR PageSize, UINT_PTR PageCount>
class RayHeap3
{
public:
	char pi[PageCount];
	char pPages[PageCount*PageSize];
	unsigned int TFree;
	unsigned int iFree;
	char *pEnd;

	RayHeap3()
	{
		TFree=0;
		iFree=PageCount;
		memset(pi, 0, PageCount);
		pEnd = &pPages[PageCount*PageSize];
	}

	char *AllocBytes()
	{
		unsigned int i = TFree;
		if(pi[i])
			i = strlen((char*)pi);

		assert(i<PageCount);

		TFree = i + ( (i+1) <PageCount);
		pi[i]=1;
		iFree--;
		return &pPages[i*PageSize];
	}

	void Dealloc(char *cptr)
	{
		assert(cptr>=pPages && cptr<pEnd);
		UINT_PTR i = ((UINT_PTR)(cptr-pPages))/PageSize;
		pi[i]=0;
		if(i<TFree)
			TFree=i;
		iFree++;
		return;
	}
};

template <UINT_PTR PageSize, UINT_PTR PageCount>
class RayHeapList2
{
public:
	vector<char> pi;
	vector<RayHeap3<PageSize+sizeof(void*),PageCount>*> heaps;
	unsigned int TFree;
	unsigned int iFree;

	RayHeapList2()
	{
		TFree=0;
		iFree=1;
		heaps.push_back(new RayHeap3<PageSize+sizeof(void*),PageCount>);
		pi.push_back(0);
	}

	RayHeapList2(unsigned int nodesize, int reserve)
	{
		assert(nodesize==PageSize);
		TFree=0;
		iFree=1;
		heaps.push_back(new RayHeap3<PageSize+sizeof(void*),PageCount>);
		pi.push_back(0);
	}

	~RayHeapList2()
	{
		for(unsigned int i=0;i<heaps.size();i++)
			delete heaps[i];
	}

	char *AllocBytes(unsigned int len)
	{
		unsigned int i = TFree;
		if(pi[i])
			i=strlen(&pi[0]);
		char *ret;
		TFree = i;

		if(heaps[i]->iFree == 1)
		{
			pi[i]=1;
			iFree--;
			if(iFree==0 && pi[heaps.size()-1]==1)
			{
				heaps.push_back(new RayHeap3<PageSize+sizeof(void*),PageCount>);
				pi.push_back(0);
				iFree++;
			}
			TFree = i + ((i+1)<heaps.size());
		}
		ret = heaps[i]->AllocBytes();
		*((RayHeap3<PageSize+sizeof(void*),PageCount>**)ret) = heaps[i];
		return ret+sizeof(void*);
	}

	void Dealloc(char * cptr)
	{
		cptr -= sizeof(void*);
		RayHeap3<PageSize+sizeof(void*),PageCount> *h = *((RayHeap3<PageSize+sizeof(void*),PageCount>**)cptr);

		h->Dealloc(cptr);

		if(h->iFree == 1)
		{
			unsigned int i = 0;
			for(;i<heaps.size() && h != heaps[i];i++);
			pi[i] = 0;
			iFree++;
		}
		else if(h->iFree == PageCount && iFree > 3)//find it and delete it
		{
			iFree--;
			unsigned int i = 0;
			for(;i<heaps.size() && h != heaps[i];i++);
			delete heaps[i];

			heaps[i] = heaps[heaps.size()-1];
			heaps.pop_back();

			pi[i] = pi[pi.size()-1];
			pi.pop_back();

			TFree=heaps.size()-1;
		}
	}
};
