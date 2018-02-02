class Allocation;


class MemObj : public SMContObj<MemObj, 2>
{
public:
	char *address;
	int free;
	Allocation *pAParent;
	MemObj *pMNext;

	MemObj(unsigned int bytes)
	{
		address = ((char*)this) + sizeof(MemObj);
		free = bytes;
	}

	char* Alloc(unsigned int bytes)
	{
		unsigned int leftovers = free - bytes;
		if(leftovers > 0 )
		{
			MemObj *ptemp = (MemObj*)(address + bytes);
			new (ptemp) MemObj(leftovers - sizeof(MemObj));
			ptemp->pAParent = pAParent;
			ptemp->pMNext = pMNext;
			pMNext = ptemp;
			this->Cont->Create( ptemp );
		}
		//pAParent->MemObjs++;

		free = -((int)bytes);

		//this->Cont->Reindex(this, 1);
		this->Cont->Del(this, 1);

		//pAParent->free -= bytes + sizeof(MemObj);

		return address;
	}

	void Dealloc()
	{
		free = free*-1;

		//this->Cont->Reindex(this, 1);
		this->Cont->Create(this, 1);
	}

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

	int SortComp(MemObj *objB, int Ind)
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
		if( free < ((int)objB) )
			return -1;
		else if( free > ((int)objB) )
			return 1;
		return 0;

	}
};


class Allocation : public ContObj<Allocation>//public SMContObj<Allocation, 2>
{
public:
	char *address;
	char *end;
	unsigned int free;

	int MemObjs;
	MemObj *pMemObj;

	Allocation(unsigned int bytes, MIndex<MemObj> &index)
	{
		address = ((char*)this) + sizeof(Allocation);
		end = address + bytes;
		free = bytes-sizeof(MemObj);
		MemObjs = 1;
		pMemObj = (MemObj*)address;
		new (pMemObj) MemObj( bytes-sizeof(MemObj) );
		pMemObj->pAParent = this;
		pMemObj->pMNext = NULL;

		index.Create(pMemObj);
	}

	/*char* NewObj(unsigned int bytes)
	{
		return 0;
	}

	void DelObj(MemObj *ptemp)
	{
	}*/

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

	/*int SortComp(Allocation *objB, int Ind)
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
	}*/
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

	MIndex<MemObj> memobjs;
	IncrBinTree<Allocation> allocs;

	RayHeap() : memobjs(2, 0)
	{
		mem = 0;
		iUsed = 0;
		lastused = 0;
		iFree = 0;
		iBlockSize = 1024*1024;
	}

	char* AllocBytes(unsigned int bytes)
	{
		MemObj *mem = NULL;
		char *ret = 0;

		mem = memobjs.Indecies[1]->GetGreaterThanOrEqualTo( (char*) (bytes) );
		if(mem != NULL)
		{
			ret = mem->Alloc(bytes);
			if(ret != NULL)
				return ret;
			mem = memobjs.Indecies[1]->GetNext(mem);
			if(mem!=NULL)
			{
				ret = mem->Alloc(bytes);
				if(ret != NULL)
					return ret;
			}
		}

		Allocation *alloc;
		unsigned int allbytes = iBlockSize;
		while(allbytes < bytes + sizeof(Allocation) + (sizeof(MemObj)*2) )
			allbytes <<= 1;
		alloc = (Allocation*) new char[allbytes];

		new (alloc) Allocation(allbytes-sizeof(Allocation), memobjs );

		ret = alloc->pMemObj->Alloc( bytes );

		return ret;
		/*Allocation *alloc = NULL;
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
		iFree += alloc->free;*/

		return 0;
	}

	void Dealloc(char* ptr)
	{
		MemObj *mem = NULL;
		//mem = memobjs.Indecies[0]->Get( ptr );
		mem = (MemObj*)(ptr-sizeof(MemObj));
		mem->Dealloc();
		/*Allocation *alloc = NULL;
		alloc = allocs.Indecies[0]->GetLessThanOrEqualTo( ptr );
		if(alloc != NULL && alloc->address <= ptr && alloc->end > ptr)
		{
			alloc->DelObj(ptr);
		}
		else
			cout << "crap....\n";*/
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