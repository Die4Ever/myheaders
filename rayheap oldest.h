/*class MemObj : public RayCont<MemObj>
{
public:
	char *address;
	char *end;

	int SortComp(MemObj *objB)
	{
		if( address < objB->address )
			return -1;
		else if( address > objB->address )
			return 1;
		return 0;
	}

	int SortComp(char *objB)
	{
		if( address < objB )
			return -1;
		else if( address > objB )
			return 1;
		return 0;
	}
};*/

class Allocation : public SMContObj<Allocation, 2>
{
public:

	struct MetaObj
	{
		unsigned int bytes;
		unsigned char used;
	};	

	char *address;
	union {
		char *end; MetaObj *metas;};
	int iMetas;
	unsigned int free;

	Allocation(unsigned int bytes)
	{
		address = ((char*)this) + sizeof(Allocation);
		end = address + bytes;
		free = bytes;
		iMetas = 0;
	}

	void InsertMeta(int InsertAfter, unsigned int bytes, unsigned char used)
	{
		MetaObj tmeta;
		MetaObj tmeta2;

		free -= sizeof(MetaObj);

		tmeta2.bytes = bytes;
		tmeta2.used = used;

		for(int i = InsertAfter-1; i >= iMetas; i--)
		{
			tmeta = metas[i];
			metas[i] = tmeta2;
			tmeta2 = tmeta;
		}
		iMetas++;
		metas[ -iMetas ] = tmeta2;
	}

	void RemoveMeta(int Remove)
	{
		MetaObj tmeta;
		MetaObj tmeta2;

		free += sizeof(MetaObj);

		tmeta2 = metas[ -iMetas];
		iMetas--;
		for(int i = -iMetas; i < Remove; i++)
		{
			tmeta = metas[i];
			metas[i] = tmeta2;
			tmeta2 = tmeta;
		}
		metas[Remove] = tmeta2;
	}

	char* NewObj(unsigned int bytes)
	{
		char *pos = address;

		for(int i=-1;i>= -iMetas;i--)
		{
			if( metas[i].used == 0 && metas[i].bytes >= bytes )
			{
				if(metas[i].bytes > bytes && free >= sizeof(MetaObj) )
				{
					InsertMeta(i, metas[i].bytes - bytes, 0);
					metas[i].bytes -= metas[i].bytes - bytes;
				}

				metas[i].used = 1;
				free -= bytes;
				return pos;
			}
			pos += metas[i].bytes;
		}
		if(free >= bytes + sizeof(MetaObj) )
		{
			iMetas++;
			metas[-iMetas].bytes = bytes;
			metas[-iMetas].used = 1;
			free -= bytes + sizeof(MetaObj);
			return pos;
		}
		else
			return 0;
	}

	void DelObj(char * obj)
	{
		char *pos = address;
		for(int i=-1;i>= -iMetas;i--)
		{
			if(pos == obj)
			{
				metas[i].used = 0;
				free += sizeof(MetaObj);
				if( i < 0 && metas[i+1].used == 0 )
				{
					metas[i+1].bytes += metas[i].bytes;
					RemoveMeta(i);
				}
				else if( i > -iMetas && metas[i-1].used == 0 )
				{
					metas[i-1].bytes += metas[i].bytes;
					RemoveMeta(i);
				}
				return;
			}
			pos += metas[i].bytes;
		}
	}

	int SortComp(Allocation *objB)
	{
		if( address < objB->address )
			return -1;
		else if( address > objB->address )
			return 1;
		return 0;
	}

	int SortComp(char *objB)
	{
		if( address < objB )
			return -1;
		else if( address > objB )
			return 1;
		return 0;
	}

	int SortComp(Allocation *objB, int Ind)
	{
		if(Ind == 0)
		{
			if( address < objB->address )
				return -1;
			else if( address > objB->address )
				return 1;
			return 0;
		}
		if( free < objB->free )
			return -1;
		else if( free > objB->free )
			return 1;
		return 0;
	}

	int SortComp(char *objB, int Ind)
	{
		if(Ind == 0)
		{
			if( address < objB )
				return -1;
			else if( address > objB )
				return 1;
			return 0;
		}
		if( free < ((unsigned int)objB) )
			return -1;
		else if( free > ((unsigned int)objB) )
			return 1;
		return 0;

	}
};

class RayHeap
{
public:
	char *mem;
	unsigned int iUsed;
	unsigned int iFree;
	Allocation *lastused;
	unsigned int iBlockSize;

	unsigned int iOverhead;//just for testing

	CriticalSection cs;

	MIndex<Allocation> allocs;

	RayHeap() : allocs(2, 0)
	{
		mem = 0;
		iUsed = 0;
		lastused = 0;
		iFree = 0;
		iBlockSize = 1024*64;
	}

	char* AllocBytes(unsigned int bytes)
	{
		Allocation *alloc = NULL;
		char * ret = 0;
		if(iFree >= bytes)
		{
			alloc = allocs.Indecies[1]->GetGreaterThanOrEqualTo( (char*) bytes);
			if(alloc != NULL && alloc->free > bytes)
			{
				while( alloc != NULL )
				{
					if(alloc->free < bytes)
						cout << "crap!\n";

					ret = alloc->NewObj(bytes);
					if(ret != 0)
					{
						allocs.Indecies[1]->Del( alloc );
						allocs.Indecies[1]->Create( alloc );
						iFree -= sizeof(Allocation::MetaObj) + bytes;
						return ret;
					}

					alloc = allocs.Indecies[1]->GetNext(alloc);
				}
			}
		}

		unsigned int allbytes = iBlockSize;
		while(allbytes < bytes + sizeof(Allocation) + sizeof(Allocation::MetaObj))
			allbytes <<= 1;
		alloc = (Allocation*) new char[allbytes];
		new (alloc) Allocation(allbytes-sizeof(Allocation) );

		ret = alloc->NewObj(bytes);
		if(ret != 0)
		{
			//allocs.Indecies[1]->Create( alloc );
			allocs.Create(alloc);
			iFree += alloc->free;
			return ret;
		}
		iFree += alloc->free;

		return 0;
	}

	void Dealloc(char* ptr)
	{
		Allocation *alloc = NULL;
		alloc = allocs.Indecies[0]->GetLessThanOrEqualTo( ptr );
		if(alloc != NULL && alloc->address <= ptr && alloc->end > ptr)
		{
			alloc->DelObj(ptr);
		}
		else
			cout << "crap....\n";
	}

	/*template <class A>
	A* Alloc(int count)
	{
		A* p = (A*)AllocBytes( sizeof(A) * count);
		new (p) A();
		return p;
		//return malloc(bytes);
	}*/

};

#define RayNew(heap, obj, args) new ((obj *)heap.AllocBytes( sizeof(obj))) obj args

#define RayNewA(heap, obj, count) new ((obj*)heap.AllocBytes( sizeof(obj) * count)) obj[count]()
#define RayDelete(heap, obj, ptr, args) ptr->~obj args, heap.Dealloc((char*)ptr), ptr=0