template<class A>
class PageObj;

int Pages=0;
template<class A>
class Page
{
public:
	int ID;
	__int64 PageSize;
	__int64 CacheSize;
	__int64 CacheUsed;
	__int64 PageUsed;
	int		Overhead;
	fstream PageFile;
	PageObj<A>* CacheHead;
	PageObj<A>* CacheTail;
	PageObj<A>* CacheTemp;

	Page(int id, __int64 csize=0,__int64 psize=0)
	{
		Overhead = sizeof(*this);
		ID = id;
		Pages++;
		PageSize = psize;
		CacheSize = csize;

		char Filename[128];
		sprintf_s(Filename,128,"page%d.dat",ID);
		PageFile.open(Filename,ios::binary | ios::out);
		PageFile.close();
		PageFile.open(Filename,ios::binary | ios::out | ios::in);
		CacheHead = CacheTail = CacheTemp = 0;
		CacheUsed = PageUsed = 0;

		if(PageSize>0)
		{
			char c[1024];
			__int64 i=0;
			memset(c,0,1024);
			for(;i<PageSize/1024;i++)
			{
				PageFile.write(c,1024);
			}
			i*=1024;
			for(;i<PageSize;i++)
			{
				PageFile.write(c,1);
			}
		}
	}

	int PageItem()
	{
		PageObj<A>* pTemp;
		for(pTemp=CacheHead; pTemp != NULL && pTemp->Obj == NULL; pTemp = pTemp->pNext);
		
		if(pTemp != NULL)
		{
			pTemp->Page();
			return 1;
		}
		return 0;
	}

	int CacheItem()
	{
		PageObj<A>* pTemp;
		for(pTemp=CacheTail; pTemp != NULL && pTemp->Obj != NULL; pTemp = pTemp->pPrev);
		
		if(pTemp != NULL)
		{
			pTemp->Cache();
			return 1;
		}
		return 0;
	}

	void Maintenance()
	{
		while(CacheUsed > CacheSize)
		{
			if(PageItem()==0)
				break;
		}

		while(CacheUsed < (CacheSize/2))
		{
			if(CacheItem()==0)
				break;
		}
	}

	PageObj<A>* AddItem()
	{
		CacheTemp = new PageObj<A>;
		CacheTemp->Parent = this;

		Overhead += sizeof(PageObj<A>);

		while((sizeof(A)+CacheUsed) > CacheSize)
		{
			if(PageItem()==0)
				break;
		}

		while((sizeof(A)+CacheUsed) < (CacheSize/2))
		{
			if(CacheItem()==0)
				break;
		}

		CacheUsed += sizeof(A);

		CacheTemp->Obj = new A;
		CacheTemp->Parent = this;
		if(CacheHead==NULL)
		{
			CacheHead = CacheTail = CacheTemp;
			return CacheTemp;
		}
		CacheTail->pNext = CacheTemp;
		CacheTemp->pPrev = CacheTail;
		CacheTail = CacheTemp;
		return CacheTemp;
	}

	PageObj<A>* GetItem(int index)
	{
		int i;
		for(CacheTemp=CacheHead, i=0;CacheTemp!=NULL;CacheTemp=CacheTemp->pNext,i++)
		{
			if(i==index)
				return CacheTemp;
		}
		return NULL;
	}
};

template<class A>
class PageObj
{
public:
	__int64 Index;
	Page<A>* Parent;
	A* Obj;
	PageObj* pNext;
	PageObj* pPrev;

	PageObj()
	{
		Index = -1;
		pNext = pPrev = 0;
		Obj = 0;
	}

	void Page()
	{
		if(Index == -1)
		{
			Parent->PageFile.seekp(0, ios::end);
			Index = Parent->PageFile.tellp();
			Parent->PageFile.write((char*)Obj, sizeof(A));
			Parent->PageUsed += sizeof(A);
		}
		else
		{
			Parent->PageFile.seekp(Index);
			Parent->PageFile.write((char*)Obj,sizeof(A));
		}
		delete Obj;
		Parent->CacheUsed -= sizeof(A);
		Obj = NULL;
	}

	void Cache()
	{
		Obj = new A;
		Parent->CacheUsed += sizeof(A);
		Parent->PageFile.seekg(Index);
		Parent->PageFile.read((char*)Obj,sizeof(A));
	}

	A* GetPtr()
	{
		if(Obj!=NULL)
			return Obj;
		Cache();
		return Obj;
	}
};