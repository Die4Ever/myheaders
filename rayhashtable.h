#pragma once

/*
TODO
-use a virtual void Transport() function instead of the constructor for copying

maybe make a sparsely allocated hash table?
	-objects won't move around in memory anymore, pointers to them are safe for the life of the object
	-maybe faster resizing, especially for large objects
	-less leaking of the abstraction
	-more ram efficient
	-could be atomic(except the memory allocations...)

persistant hash table
	-sparse or dense?
*/

template <class A, class Key, unsigned int padding=0>
class HashTableObj
{
public:
	A				Obj;
	char			Padding[padding];
	unsigned int	Hash;
	HashTableObj<A,Key> *pNext;

	void SetUnused()
	{
		pNext=(HashTableObj<A,Key> *)0x1;
	}

	void DeAlloc()
	{
		if(pNext != (HashTableObj<A,Key> *)0x1)
		{
			this->~HashTableObj();
			return;
		}
	}

	HashTableObj() : Obj()
	{
		pNext=NULL;
	}

	//HashTableObj(A &obj, unsigned int hash) : Obj(obj)
	HashTableObj(A &obj, unsigned int hash) : Obj()
	{
		//this->~HashTableObj();
		obj.Transport(&Obj);
		pNext=NULL;
		Hash = hash;
	}

	HashTableObj(Key key, unsigned int hash) : Obj(key)
	{
		pNext=NULL;
		Hash = hash;
	}

	~HashTableObj()
	{
		if(pNext > (HashTableObj<A,Key> *)0x1)
			delete pNext;

		pNext=(HashTableObj<A,Key> *)0x1;
	}

};

template <class A, class Key>
class HashTableObj<A,Key,0>
{
public:
	A				Obj;
	unsigned int	Hash;
	HashTableObj<A,Key> *pNext;

	void SetUnused()
	{
		pNext=(HashTableObj<A,Key> *)0x1;
	}

	void DeAlloc()
	{
		if(pNext != (HashTableObj<A,Key> *)0x1)
		{
			this->~HashTableObj();
			return;
		}
		//if(pNext > (HashTableObj<A,Key> *)0x1)
		//	delete pNext;

		//pNext=(HashTableObj<A,Key> *)0x1;
	}

	HashTableObj() : Obj()
	{
		pNext=NULL;
	}

	//HashTableObj(A &obj, unsigned int hash) : Obj(obj)
	HashTableObj(A &obj, unsigned int hash) : Obj(obj)
	{
		//auto *p=this->pNext;
		//pNext=NULL;
		//this->~HashTableObj();
		//pNext=p;
		//obj.Transport(&Obj);
		pNext=NULL;
		Hash = hash;
	}

	HashTableObj(Key key, unsigned int hash) : Obj(key)
	{
		pNext=NULL;
		Hash = hash;
	}

	~HashTableObj()
	{
		if(pNext > (HashTableObj<A,Key> *)0x1)
			delete pNext;

		pNext=(HashTableObj<A,Key> *)0x1;
	}

};


template <class A, class Key, class LargeObj=A, bool Static=0>
class HashTable : public Cont2
{
public:
	typedef HashTableObj<A,Key,sizeof(LargeObj)-sizeof(A)> Node;
	unsigned int Objects;
	unsigned int Slots;

	Node *Table;

private:
	HashTable(const HashTable &old)
	{
		cerr << "WTF!\n";
	}

	HashTable operator=(const HashTable &old)
	{
		cerr << "WTF!\n";
	}

protected:
	void Resize(unsigned int newsize)
	{
		//cout << "Resizing\n";
		Node *oldtable = Table;
		Table = (Node*)new char[newsize*sizeof(Node)];
		for(unsigned int i=0;i<newsize;i++)
			Table[i].SetUnused();

		unsigned int oldsize = Slots;
		Slots = newsize;
		//unsigned int OldObjects = Objects;
		Objects = 0;

		for(Node *p = oldtable; p < &oldtable[oldsize]; p++)
		{
			if(p->pNext != (Node*)0x1)
			{
				for(Node *p2=p; p2!=NULL; p2=p2->pNext)
				{
					Create(p2->Obj);//, p2->Hash);
				}
			}
		}

		//Objects = OldObjects;
		for(unsigned int i=0;i<oldsize;i++)
			oldtable[i].DeAlloc();

		delete[] ((char*)oldtable);

		//cout << "Done Resizing\n";
	}

public:

	~HashTable()
	{
		Clear();
		delete[] ((char*)Table);
		Table=NULL;
	}

	HashTable& operator=(HashTable &&old)
	{
		Table=old.Table;
		old.Table=NULL;
		Objects=old.Objects;
		Slots=old.Slots;
		old.Slots=0;
		return *this;
	}

	HashTable(HashTable &&old)
	{
		Table=old.Table;
		old.Table=NULL;
		Objects=old.Objects;
		Slots=old.Slots;
		old.Slots=0;
	}

	HashTable(unsigned int reserve=2)
	{
		Table = (Node*)new char[reserve*sizeof(Node)];
		for(unsigned int i=0;i<reserve;i++)
			Table[i].SetUnused();
		Slots = reserve;
		Objects = 0;
	}

	template <class B>
	A *Create(B &tobj)
	{
		typedef HashTableObj<B,Key,sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) == sizeof(Node));
		if(!Static)
		{
			if(Objects > (Slots)-(Slots>>2))
			{
				Resize(Slots*2);
			}
		}

		Objects++;
		unsigned int hash = tobj.Hash();
		unsigned int slot = hash%Slots;
		if(Table[slot].pNext == (Node*)0x1)
		{
			//Table[slot] = NodeB(tobj, hash);
			new(&Table[slot]) NodeB(tobj, hash);
			return (B*)&Table[slot];
		}
		else
		{
			Node *p = &Table[slot];
			for(; p->pNext!=NULL; p=p->pNext);
			p->pNext = (Node*)new NodeB(tobj, hash);
			return (B*)p->pNext;
		}
	}

	template <class B>
	A *CreateKey(const Key key)
	{
		typedef HashTableObj<B,Key,sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) == sizeof(Node));
		if(!Static)
		{
			if(Objects > (Slots)-(Slots>>2))
			{
				Resize(Slots*2);
			}
		}

		Objects++;
		unsigned int hash = B::Hash(key);
		unsigned int slot = hash%Slots;
		if(Table[slot].pNext == (Node*)0x1)
		{
			//Table[slot] = NodeB(tobj, hash);
			new(&Table[slot]) NodeB(key, hash);
			return (B*)&Table[slot];
		}
		else
		{
			Node *p = &Table[slot];
			for(; p->pNext!=NULL; p=p->pNext);
			p->pNext = new NodeB(key, hash);
			return (B*)p->pNext;
		}
	}

	/*template <class B>
	A *Create(B &tobj, unsigned int hash)
	{
		typedef HashTableObj<B,Key,sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) != sizeof(Node));
		if(!Static)
		{
			if(Objects > (Slots)-(Slots>>2))
			{
				Resize(Slots*2);
			}
		}

		Objects++;
		unsigned int slot = hash%Slots;
		if(Table[slot].pNext == (Node*)0x1)
		{
			//Table[slot] = NodeB(tobj, hash);
			new(&Table[slot]) NodeB(tobj, hash);
			return (B*)&Table[slot];
		}
		else
		{
			Node *p = &Table[slot];
			for(; p->pNext!=NULL; p=p->pNext);
			p->pNext = new NodeB(tobj, hash);
			return (B*)p->pNext;
		}
	}

	template <class B>
	A *CreateKey(const Key key, unsigned int hash)
	{
		typedef HashTableObj<B,Key,sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) != sizeof(Node));
		if(!Static)
		{
			if(Objects > (Slots)-(Slots>>2))
			{
				Resize(Slots*2);
			}
		}

		Objects++;
		unsigned int slot = hash%Slots;
		if(Table[slot].pNext == (Node*)0x1)
		{
			//Table[slot] = NodeB(tobj, hash);
			new(&Table[slot]) NodeB(key, hash);
			return (B*)&Table[slot];
		}
		else
		{
			Node *p = &Table[slot];
			for(; p->pNext!=NULL; p=p->pNext);
			p->pNext = new NodeB(key, hash);
			return (B*)p->pNext;
		}
	}*/

	A *GetFirst()
	{
		Node *p = Table;
		for(; ; p++)
		{
			if(p>= &Table[Slots])
				return NULL;
			if(p->pNext !=(Node*)0x1)
				return (A*)p;
		}
		//return (A*)p;
	}

	A *GetNext(const A *obj)
	{
		Node *p = (Node*)obj;
		if(p->pNext > (Node*)0x1)
		{
			return (A*)p->pNext;
		}
		p = &Table[p->Hash%Slots];
		for(p++; ; p++)
		{
			if(p>= &Table[Slots])
				return NULL;
			if(p->pNext!=(Node*)0x1)
				return (A*)p;
		}
		//return (A*)p;
	}

	A *Get(Key key)
	{
		unsigned int hash = A::Hash(key);
		unsigned int slot = hash%Slots;

		if(Table[slot].pNext == (Node*)0x1)
		{
			return NULL;
		}
		else
		{
			Node *p = &Table[slot];
			for(; p!=NULL; p=p->pNext)
			{
				//if(p->Hash == hash)
				{
					if(p->Obj.SortComp(key) == 0)
						return (A*)p;
				}
			}
			return (A*)p;
		}
	}

	A *Get(Key key, unsigned int slot)
	{
		//unsigned int hash = A::Hash(key);
		//unsigned int slot = hash%Slots;

		if(Table[slot].pNext == (Node*)0x1)
		{
			return NULL;
		}
		else
		{
			Node *p = &Table[slot];
			for(; p!=NULL; p=p->pNext)
			{
				//if(p->Hash == hash)
				{
					if(p->Obj.SortComp(key) == 0)
						return (A*)p;
				}
			}
			return (A*)p;
		}
	}

	void GetAll(const Key key, vector<A*> &ret)
	{
		unsigned int hash = A::Hash(key);
		unsigned int slot = hash%Slots;

		if(Table[slot].pNext != (Node*)0x1)
		{
			Node *p = &Table[slot];
			for(; p!=NULL; p=p->pNext)
			{
				if(p->Hash == hash)
				{
					if(p->Obj.SortComp(key) == 0)
						ret.push_back((A*)p);
				}
			}
		}
	}

	//program this....
	void Delete(A *obj)
	{
		Node *p = (Node*)obj;

		if(p >= &Table[0] && p < &Table[Slots])
		{
			Node *p2 = NULL;
			Node *p3 = NULL;
			if(p->pNext > (Node*)0x1)
			{
				p2 = p->pNext;
				p3=p2->pNext;
				p2->pNext = NULL;
				p->pNext = NULL;
			}
			p->DeAlloc();

			if(p2)
			{
				new(p) Node(p2->Obj, p2->Hash);
				p->pNext = p3;
				delete p2;
			}
		}
		else
		{
			unsigned int hash = p->Hash;//obj->Hash();
			unsigned int slot = hash%Slots;

			Node *p2 = &Table[slot];
			for(; p2 && p2->pNext!=p; p2=p2->pNext);
			if(p2 && p2->pNext==p)
				p2->pNext=p->pNext;
			p->pNext=NULL;
			delete p;
		}
		Objects--;

		if(!Static)
		{
			if(Objects < (Slots>>3))
			{
				Resize(Slots>>2);
			}
		}
	}

	void Clear()
	{
		for(unsigned int i=0;i<Slots;i++)
		{
			Table[i].DeAlloc();
			//if(Table[i].pNext != (Node*)0x1)
			//	Table[i].~Node();
		}
		Objects=0;
	}

	A *ReIndex(A *obj)
	{
		return NULL;
	}

};
