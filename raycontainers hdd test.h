/*lol, this gets crazy, pay attention
when making a class that you want to store it's objects in a linked list, make it inherit from LLObj<the class name>, that way it gets Next and Prev pointers of it's own type...
then to make a list of that object, make an object of LinkedList<the class type> so you have a LinkedList of the original class
example:
class Character : public LLObj<Character>
{
//....
};
LinkedList<Character> CharacterList;

the only case you might need the virtual destructor for the LLObj is if you make a general linked list of type LLObj with inherited class objects in the list
virtual functions will get more use, the bytes(4 or 8) used for the virtual function table will not be wasted!
*/

/*
-New Container Ideas
-Binary Tree with collision support(using linked lists maybe?)
-Multidimensional trees(quad, oct,...)(it works if you just do an array of binary trees, but you have to manually add items to each tree)
-an ordered binary tree and linked list combo(require a different kind of ContObj?)
-N-Tree (each item contains a linked list)
*/

/*
TODO
-a reverse OrderedGet() on my trees, instead of going to the left first, start from the right, and instead of starting with item 0 it would start with item (Objects - 1)
-automatically use this method if you are requesting an item with an index > Objects/2
-also use a seperate function that calls OrderedGet indirectly, so you don't need to pass the index number by reference and it can decide which direction to travel

-Add a Linked List Index in addition to the current Binary Tree Index, for fast sequential Indecies, also add support for mixing the 2 index types
-maybe I can make a string type with a Linked List index on the words and a full text index in a binary tree

-some cooler functions(sql style) like LessThan() and GreaterThan() functions(EqualTo() is already done)
-GetClosest() function...goes at every step it checks which is closest, Current, pLeft or pRight
-will need a virtual scoring function for this which returns a closeness score(probably a float or a double, signed so my function can tell which direction)
-maybe also have a GetNextLargest() and GetNextSmallest() (these could be done without distance functions, go left 1 and right all the way down for next smallest...)
-GetNextLargest() would be the same as GetClosest(), except only where the distance is positive
-GetNextSmallest() would return the largest one where the distance is negative
-all should use a Start pointer, so you can iterate over them
-nevermind about LessThan() and GreaterThan()...can just do...
Temp = List.OrderedGet(0);
while(Temp!=NULL && Temp->i < a)
{ ... }

-or maybe...
Temp = List.GetClosest( i );
if(Temp!=NULL) Temp=Temp->pLeft;


*/

//template <class A>
//class ContObj;

#include "binary.h"

int log2(int a)
{
	int i=31;
	for(; i>1 && (a & (1<<i)) == 0; i--);
	return i;
}

template <class A>
class RayCont
{
public:
	union{A *pHead,*pRoot;};
	A*		pTail;
	A*		pTemp;
	int		iSorted;
	int		Objects;
	RayCont<A> *pParent;

	RayCont(){Objects=0;pParent=0;pRoot=0;pTail=0;}
	virtual ~RayCont(){}
	virtual void	Create(A* currobj) = 0;
	virtual void	Create(A* currobj, int ind) { Create(currobj); }
	virtual void	Del(A* currobj) = 0;
	virtual void	Del(A* currobj, int ind) {Del(currobj);}
	virtual A*		Get(char* tempname) = 0;
	virtual A*		Get(int &index, A *Start=0) = 0;
	virtual A*		GetNext(char* tempname, A *Start){ return NULL; }
	virtual A*		GetNext(A *Start){ return NULL; }
	virtual A*		GetPrev(A *Start){ return NULL; }
	virtual A*		OrderedGet(int &index, A *Start=0) = 0;
	virtual A*		CopyObj(A* obj) = 0;
	virtual char*	DisplayList() = 0;
	virtual void	Sort() = 0;
	virtual void	Clear(A *Start=0) = 0;

	virtual void	Reindex(A* currobj) { Del(currobj); Create(currobj); }
	virtual void	Reindex(A* currobj, int ind) { Del(currobj, ind); Create(currobj, ind); }

	virtual A*		GetGreaterThan(char *tempname) { return NULL; }
	virtual A*		GetGreaterThanOrEqualTo(char *tempname) { return NULL; }
	virtual A*		GetLessThan(char *tempname) {return NULL; }
	virtual A*		GetLessThanOrEqualTo(char *tempname) {return NULL; }
};

/*template <class A>
class ContObj
{
public:
virtual ~ContObj() {}
virtual A* RetThis() = 0;
virtual void Create() = 0;
virtual void Del() = 0;
virtual char* MoreInfo() = 0;
virtual int SortComp(A* objB) = 0;
union{ A *pPrev,*pLeft;};
union{ A *pNext,*pRight;};
union{ A *pHead,*pRoot;};
A *pTail;
A *pParent;
A *pTemp;
int iSorted;
};*/



//template <class A>
//class ContObj;

template <class A>
class LinkedList : public RayCont<A>
{
public:
	virtual void	Create(A* currobj);
	virtual void	Del(A* currobj);
	virtual A*		Get(char* tempname);
	virtual A*		Get(int &index, A *Start=0);
	virtual A*		OrderedGet(int &index, A *Start=0);
	virtual A*		CopyObj(A* obj);
	virtual char*	DisplayList();
	virtual void	Sort();
	virtual void	Clear(A *Start=0);

	LinkedList();
	virtual ~LinkedList() { Clear();}
};

template <class A>
class ContObj
{
public:
	char	*name;
	union{ A *pPrev,*pLeft;};
	union{ A *pNext,*pRight;};
	A	*pParent;
	RayCont<A>*	Cont;

	virtual ~ContObj()
	{
		delete[] name;
		name = 0;
		Del();
	}

	ContObj()
	{
		pParent = NULL;
		pPrev = pNext = NULL;
		Cont = NULL;
		name = NULL;
	}

	A* RetThis()
	{
		return (A*)this;
	}

	void Create()
	{
		if(Cont != NULL)
			Cont->Create((A*)this);
	}

	void Del()
	{
		if(Cont != NULL)
			Cont->Del((A*)this);
	}

	virtual char* MoreInfo()
	{//this is going to have to return a char*
		//cout << "More info on "<<name<< "...\n";
		return 0;
	}

	/*virtual */int SortComp(A* objB)
	{
		if(name==NULL)
			return -1;
		if(objB->name == NULL)
			return 1;
		return strcmp(name, objB->name);
	}

	/*virtual */int SortComp(char *objB)
	{
		if(name==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name, objB);
	}
};

template <class A>
class MIndex;

template <class A>
class Index;

template <class A>
class MContObj//MultiIndex
{
public:
	char	**name;
	union{ A **pPrev,**pLeft;};
	union{ A **pNext,**pRight;};
	int iIndecies;

	A	**pParent;
	RayCont<A>*	Cont;

	virtual ~MContObj()
	{
		Del();
		for(int i=0;i<iIndecies;i++)
		{
			delete[] name[i];
		}
		delete[] name;
		delete[] pLeft;
		delete[] pRight;
		delete[] pParent;
		name = 0;
	}

	MContObj()
	{
		iIndecies = 0;
		pParent = NULL;
		pPrev = pNext = NULL;
		pLeft = NULL;
		pRight = NULL;
		name = NULL;
		pParent = NULL;

		Cont = NULL;
	}

	MContObj(int Ind)
	{
		iIndecies = Ind;
		pParent = NULL;
		pPrev = pNext = NULL;
		pLeft = new A*[iIndecies];
		pRight = new A*[iIndecies];
		name = new char*[iIndecies];
		pParent = new A*[iIndecies];
		for(int i=0;i<iIndecies;i++)
		{
			pLeft[i] = NULL;
			pRight[i] = NULL;
			name[i] = NULL;
			pParent[i] = NULL;
		}

		Cont = NULL;
	}

	void SetIndexCount(int Ind)
	{
		Del();
		for(int i=0;i<iIndecies;i++)
		{
			delete[] name[i];
		}
		delete[] name;
		delete[] pLeft;
		delete[] pRight;
		delete[] pParent;
		name = 0;

		iIndecies = Ind;
		pParent = NULL;
		pPrev = pNext = NULL;
		pLeft = new A*[iIndecies];
		pRight = new A*[iIndecies];
		name = new char*[iIndecies];
		pParent = new A*[iIndecies];
		for(int i=0;i<iIndecies;i++)
		{
			pLeft[i] = NULL;
			pRight[i] = NULL;
			name[i] = NULL;
			pParent[i] = NULL;
		}

		Cont = NULL;
	}

	A* RetThis()
	{
		return (A*)this;
	}

	void Create()
	{
		if(Cont != NULL)
			Cont->Create((A*)this);
	}

	void Del()
	{
		if(Cont != NULL)
			Cont->Del((A*)this);
		/*for(int i=0;i<iDimensions;i++)
		{
		if(Cont != NULL)
		Cont->Del((A*)this, i);
		}*/
	}

	/*void Del(int Ind)
	{
	if(Cont != NULL)
	Cont->Del((A*)this, Ind);
	}*/

	virtual char* MoreInfo()
	{//this is going to have to return a char*
		//cout << "More info on "<<name<< "...\n";
		return 0;
	}

	/*virtual */int SortComp(A* objB)
	{
		if(name==NULL || name[0]==NULL)
			return -1;
		if(objB->name == NULL || objB->name[0]==NULL)
			return 1;
		return strcmp(name[0], objB->name[0]);
	}

	/*virtual */int SortComp(char *objB)
	{
		if(name==NULL || name[0]==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name[0], objB);
	}

	/*virtual */int SortComp(A* objB, int Ind)
	{
		if(name==NULL||name[Ind]==NULL)
			return -1;
		if(objB->name == NULL||objB->name[Ind]==NULL)
			return 1;
		return strcmp(name[Ind], objB->name[Ind]);
	}

	/*virtual */int SortComp(char *objB, int Ind)
	{
		if(name==NULL || name[Ind]==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name[Ind], objB);
	}
};

template <class A, int iIndecies>
class SMContObj//Static MultiIndex
{
public:
	char	*name[iIndecies];
	union{ A *pPrev[iIndecies],*pLeft[iIndecies];};
	union{ A *pNext[iIndecies],*pRight[iIndecies];};
	//int iIndecies;

	A	*pParent[iIndecies];
	RayCont<A>*	Cont;

	virtual ~SMContObj()
	{
		Del();
		for(int i=0;i<iIndecies;i++)
		{
			delete[] name[i];
			pLeft[i] = NULL;
			pRight[i] = NULL;
			name[i] = NULL;
			pParent[i] = NULL;
		}
	}

	SMContObj()
	{
		for(int i=0;i<iIndecies;i++)
		{
			pLeft[i] = NULL;
			pRight[i] = NULL;
			name[i] = NULL;
			pParent[i] = NULL;
		}

		Cont = NULL;
	}

	SMContObj(int Ind)
	{
		for(int i=0;i<iIndecies;i++)
		{
			pLeft[i] = NULL;
			pRight[i] = NULL;
			name[i] = NULL;
			pParent[i] = NULL;
		}

		Cont = NULL;
	}

	void SetIndexCount(int Ind)
	{
	}

	A* RetThis()
	{
		return (A*)this;
	}

	void Create()
	{
		if(Cont != NULL)
			Cont->Create((A*)this);
	}

	void Del()
	{
		if(Cont != NULL)
			Cont->Del((A*)this);
	}

	void Del(int ind)
	{
		if(Cont != NULL)
			Cont->Del((A*)this, ind);
	}

	/*void Del(int Ind)
	{
	if(Cont != NULL)
	Cont->Del((A*)this, Ind);
	}*/

	virtual char* MoreInfo()
	{//this is going to have to return a char*
		//cout << "More info on "<<name<< "...\n";
		return 0;
	}

	/*virtual */int SortComp(A* objB)
	{
		if(name[0]==NULL)
			return -1;
		if(objB->name[0]==NULL)
			return 1;
		return strcmp(name[0], objB->name[0]);
	}

	/*virtual */int SortComp(char *objB)
	{
		if(name[0]==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name[0], objB);
	}

	/*virtual */int SortComp(A* objB, int Ind)
	{
		if(name[Ind]==NULL)
			return -1;
		if(objB->name[Ind]==NULL)
			return 1;
		return strcmp(name[Ind], objB->name[Ind]);
	}

	/*virtual */int SortComp(char *objB, int Ind)
	{
		if(name[Ind]==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name[Ind], objB);
	}
};



template <class A>
LinkedList<A>::LinkedList()
{
	this->iSorted = 0;
	this->pHead = this->pTail = this->pTemp = 0;
}

template <class A>
void LinkedList<A>::Clear(A *Start)
{
	while(this->pHead!=NULL)
		delete this->pHead;
}

template <class A>
void LinkedList<A>::Create(A* currobj)
{
	if(currobj==NULL)
		return;
	this->Objects++;
	currobj->Cont = this;
	if(this->iSorted==0)
	{
		if(this->pHead==NULL)
			this->pHead = this->pTail = currobj;
		else
		{
			this->pTail->pNext = currobj;
			currobj->pPrev = this->pTail;
			this->pTail = currobj;
		}
	}
	else
	{
		A* pTemp;
		//add it with an insert sort, I thought about a quick sort, but it's a linked list, it would have to walk through every item anyways and in some cases even go back
		if(this->pHead==NULL)
		{
			this->pHead = this->pTail = currobj;
			return;
		}
		for(pTemp=this->pHead; pTemp != NULL; pTemp = pTemp->pNext)
		{
			if(pTemp->SortComp(currobj) > 0)//if currobj is supposed to come before pTemp
			{
				if(pTemp == this->pHead)
					this->pHead = currobj;
				if(pTemp->pPrev != NULL)
					pTemp->pPrev->pNext = currobj;
				currobj->pPrev = pTemp->pPrev;
				pTemp->pPrev = currobj;
				currobj->pNext = pTemp;
				return;
			}
		}
		this->pTail->pNext = currobj;
		currobj->pPrev = this->pTail;
		this->pTail = currobj;
	}
}

template <class A>
void LinkedList<A>::Del(A* currobj)
{
	this->Objects--;
	if(currobj->pNext!=NULL)
		currobj->pNext->pPrev = currobj->pPrev;
	if(currobj->pPrev!=NULL)
		currobj->pPrev->pNext = currobj->pNext;
	if(this->pHead==currobj)
		this->pHead = currobj->pNext;
	if(this->pTail==currobj)
		this->pTail = currobj->pPrev;
}

template <class A>
A* LinkedList<A>::Get(char* tempname)
{
	A* pTemp = this->pTemp;
	if(pTemp!=NULL && !pTemp->SortComp(tempname))
		return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL && pTemp->SortComp(tempname) ;pTemp=pTemp->pNext);
	//if(!pTemp->SortComp(tempname))
	return pTemp;
	return NULL;
}

template <class A>
A* LinkedList<A>::Get(int &index, A *Start)
{
	int i;
	int dista,distb;

	A* pTemp = this->pTemp;

	if(Start==0)
		Start=this->pHead;
	else
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext,i++)
			if(i==index)
				return pTemp;
		return NULL;
	}

	if(index>=this->Objects || index<0)
		return NULL;

	dista = index;
	distb = this->Objects - index;

	if(dista <= distb)
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext,i++)
			if(i==index)
				return pTemp;
	}
	else
	{
		for(pTemp=this->pTail, i=this->Objects-1;pTemp!=NULL;pTemp=pTemp->pPrev,i--)
			if(i==index)
				return pTemp;
	}
	return NULL;
}

template <class A>
A* LinkedList<A>::OrderedGet(int &index, A *Start)
{
	int i;
	int dista,distb;

	A* pTemp = this->pTemp;

	if(Start==0)
		Start=this->pHead;
	else
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext,i++)
			if(i==index)
				return pTemp;
		return NULL;
	}

	if(index>=this->Objects || index<0)
		return NULL;

	dista = index;
	distb = this->Objects - index;

	if(dista <= distb)
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext,i++)
			if(i==index)
				return pTemp;
	}
	else
	{
		for(pTemp=this->pTail, i=this->Objects-1;pTemp!=NULL;pTemp=pTemp->pPrev,i--)
			if(i==index)
				return pTemp;
	}
	return NULL;
}

template <class A>
A* LinkedList<A>::CopyObj(A* obj)
{
	A* pTemp;
	pTemp = new A(*obj);
	pTemp->pNext = NULL;
	pTemp->pPrev = NULL;
	Create(pTemp);
	return pTemp;
}

template <class A>
char* LinkedList<A>::DisplayList()
{
	int i=0;
	A* pTemp = this->pTemp;
	for(pTemp=this->pHead;pTemp!=NULL;pTemp=pTemp->pNext)
	{
		i++;
		//cout << "("<<i<<") "<<pTemp->name<<"\n";
	}
	return 0;
	/*cout << "If you would like more info on something, type in it's name, otherwise just press Enter\n";
	cin >> tempstring;
	pTemp = Get(tempstring);
	if(pTemp !=NULL)
	{
	pTemp->MoreInfo();
	}
	cout<<"\n\n";*/
}

template <class A>
void LinkedList<A>::Sort()
{
	//pTemp
	A* pTemp = this->pTemp;
	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		if(pTemp->pNext != NULL && pTemp->SortComp(pTemp->pNext)>0)
		{
			//switch the two in the linked list
			if(this->pHead == pTemp)
				this->pHead = pTemp->pNext;
			if(this->pTail == pTemp->pNext)
				this->pTail = pTemp;
			if(pTemp->pPrev!=NULL)
				pTemp->pPrev->pNext = pTemp->pNext;
			if(pTemp->pNext->pNext!=NULL)
				pTemp->pNext->pNext->pPrev = pTemp;
			pTemp->pNext->pPrev = pTemp->pPrev;
			pTemp->pPrev = pTemp->pNext;
			pTemp->pNext = pTemp->pPrev->pNext;
			pTemp->pPrev->pNext = pTemp;

			pTemp = pTemp->pPrev->pPrev;
			if(pTemp==NULL)
				pTemp = this->pHead;
		}
		else
			pTemp = pTemp->pNext;
	}
}


//template <class A>
//class HashObj;

template <class A>
class Hash: public RayCont<A>
{
public:
	virtual void	Create(A* currobj);
	virtual void	Del(A* currobj);
	virtual A*		Get(char* tempname);
	virtual A*		GetNext(char* tempname, A *Start);
	virtual A*		GetNext(A *Start);
	virtual A*		GetPrev(A *Start);
	virtual A*		Get(int &index, A *Start=0);
	virtual A*		OrderedGet(int &index, A *Start=0);
	virtual A*		CopyObj(A* obj);
	virtual char*	DisplayList();
	virtual void	Sort();
	virtual void	Clear(A *Start=0);
	virtual void	RotLeft(A *Obj);
	virtual void	RotRight(A *Obj);

	virtual A*		GetLessThan(char* tempname);
	virtual A*		GetGreaterThan(char* tempname);

	virtual A*		GetLessThanOrEqualTo(char* tempname);
	virtual A*		GetGreaterThanOrEqualTo(char* tempname);

	Hash();
	virtual ~Hash() { Clear(); }
};

template <class A>
Hash<A>::Hash()
{
	this->iSorted = 1;
	this->pRoot = this->pTail = this->pTemp = 0;
}

template <class A>
void Hash<A>::Clear(A *Start)
{
	if(Start==0)
	{
		Start=this->pRoot;
		while(this->pRoot != NULL)
			delete this->pRoot;
		return;
	}

	if(Start==0)
		return;

	if(Start->pLeft!=0)
		Clear(Start->pLeft);
	if(Start->pRight!=0)
		Clear(Start->pRight);
	if(Start == this->pRoot)
		this->pRoot = NULL;
	if(Start == this->pTail)
		this->pTail = NULL;
	delete Start;
}

template <class A>
void Hash<A>::Create(A* currobj)
{
	if(currobj==NULL)
		return;

	this->Objects++;
	currobj->Cont = this;
	if(this->pRoot==NULL)
	{
		this->pRoot = this->pTail = currobj;
		return;
	}
	this->pTail = currobj;
	for(A *pTemp=this->pRoot; pTemp != NULL;)
	{
		if(pTemp->SortComp(currobj) < 0)//right
		{
			stepcount++;
			if(pTemp->pRight == NULL)
			{
				pTemp->pRight = currobj;
				currobj->pParent = pTemp;
				return;
			}
			pTemp = pTemp->pRight;
		}
		else//left
		{
			stepcount++;
			if(pTemp->pLeft == NULL)
			{
				pTemp->pLeft = currobj;
				currobj->pParent = pTemp;
				return;
			}
			pTemp = pTemp->pLeft;
		}
	}
}

template <class A>
void Hash<A>::Del(A* currobj)
{
	if(currobj->pLeft == NULL && currobj->pRight == NULL)
	{
		if(this->pTemp == currobj)
			this->pTemp = currobj->pParent;
		if(this->pRoot == currobj)
			this->pRoot = NULL;
		if(this->pTail == currobj)
			this->pTail = currobj->pParent;
		if(currobj->pParent != NULL)
		{
			if(currobj->pParent->pLeft == currobj)
				currobj->pParent->pLeft = NULL;
			else
				currobj->pParent->pRight = NULL;
		}
	}
	else if( currobj->pLeft == NULL || currobj->pRight == NULL)
	{
		if(currobj->pLeft == NULL)
		{
			if(this->pTemp == currobj)
				this->pTemp = currobj->pRight;
			if(this->pRoot == currobj)
				this->pRoot = currobj->pRight;
			if(this->pTail == currobj)
				this->pTail = currobj->pRight;

			if(currobj->pParent != NULL)
			{
				if(currobj->pParent->pLeft == currobj)
					currobj->pParent->pLeft = currobj->pRight;
				else
					currobj->pParent->pRight = currobj->pRight;
			}

			currobj->pRight->pParent = currobj->pParent;
		}
		else
		{
			if(this->pTemp == currobj)
				this->pTemp = currobj->pLeft;
			if(this->pRoot == currobj)
				this->pRoot = currobj->pLeft;
			if(this->pTail == currobj)
				this->pTail = currobj->pLeft;

			if(currobj->pParent != NULL)
			{
				if(currobj->pParent->pLeft == currobj)
					currobj->pParent->pLeft = currobj->pLeft;
				else
					currobj->pParent->pRight = currobj->pLeft;
			}

			currobj->pLeft->pParent = currobj->pParent;
		}
	}
	else
	{
		A *TempItem;
		if(rand()&1)
		{
			TempItem = currobj->pLeft;
			while(TempItem->pRight != NULL)
			{
				TempItem = TempItem->pRight;
			}
		}
		else
		{
			TempItem = currobj->pRight;
			while(TempItem->pLeft != NULL)
			{
				TempItem = TempItem->pLeft;
			}
		}

		Del(TempItem);

		TempItem->pLeft = currobj->pLeft;
		TempItem->pRight = currobj->pRight;
		TempItem->pParent = currobj->pParent;

		if(TempItem->pLeft!=NULL)
			TempItem->pLeft->pParent = TempItem;
		if(TempItem->pRight!=NULL)
			TempItem->pRight->pParent = TempItem;

		if(currobj->pParent != NULL)
		{
			if(currobj->pParent->pLeft == currobj)
				currobj->pParent->pLeft = TempItem;
			else
				currobj->pParent->pRight = TempItem;
		}

		if(this->pTemp == currobj)
			this->pTemp = TempItem;
		if(this->pRoot == currobj)
			this->pRoot = TempItem;
		if(this->pTail == currobj)
			this->pTail = TempItem;

		return;
	}
	this->Objects--;
}

template <class A>
A* Hash<A>::GetNext(A *Start)
{
	if(Start == NULL)
		return NULL;

	A *tobj = NULL;
	if(Start->pRight != NULL)
	{
		tobj = Start->pRight;
		while(tobj->pLeft != NULL)
			tobj = tobj->pLeft;
		return tobj;
	}

	if(Start->pParent != NULL)
	{
		A *tobj2 = Start;
		tobj = tobj2->pParent;

		while( tobj2 != tobj->pLeft )
		{
			tobj2 = tobj;
			tobj = tobj->pParent;
			if(tobj==NULL)
				return NULL;
		}

		return tobj;
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetPrev(A *Start)
{
	if(Start == NULL)
		return NULL;

	A *tobj = NULL;
	if(Start->pLeft != NULL)
	{
		tobj = Start->pLeft;
		while(tobj->pRight != NULL)
			tobj = tobj->pRight;
		return tobj;
	}

	if(Start->pParent != NULL)
	{
		A *tobj2 = Start;
		tobj = tobj2->pParent;

		while( tobj != NULL && tobj2 != tobj->pRight )
		{
			tobj2 = tobj;
			tobj = tobj->pParent;
			if(tobj==NULL)
				return NULL;
		}

		return tobj;
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetGreaterThan(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		if(i < 0)//right
		{
			if(pTemp->pRight == NULL)
			{
				pTemp = GetNext(pTemp);
				/*while( pTemp->SortComp(tempname) < 0)
				{
					pTemp = GetNext(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}
			else
				pTemp = pTemp->pRight;
		}
		else if(i > 0)
		{
			if(pTemp->pLeft == NULL)
				return pTemp;

			pTemp = pTemp->pLeft;
		}
		else
		{
			pTemp = GetPrev(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetLessThan(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		if(i < 0)//right
		{
			if(pTemp->pRight == NULL)
				return pTemp;
			else
				pTemp = pTemp->pRight;
		}
		else if(i > 0)
		{
			if(pTemp->pLeft == NULL)
			{
				pTemp = GetPrev(pTemp);
				/*while( pTemp->SortComp(tempname) > 0)
				{
					pTemp = GetPrev(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}

			pTemp = pTemp->pLeft;
		}
		else
		{
			pTemp = GetNext(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetGreaterThanOrEqualTo(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		if(i < 0)//right
		{
			if(pTemp->pRight == NULL)
			{
				pTemp = GetNext(pTemp);
				/*while( pTemp->SortComp(tempname) < 0)
				{
					pTemp = GetNext(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}
			else
				pTemp = pTemp->pRight;
		}
		else if(i > 0)
		{
			if(pTemp->pLeft == NULL)
				return pTemp;

			pTemp = pTemp->pLeft;
		}
		else
		{
			//pTemp = GetPrev(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetLessThanOrEqualTo(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		if(i < 0)//right
		{
			if(pTemp->pRight == NULL)
				return pTemp;
			else
				pTemp = pTemp->pRight;
		}
		else if(i > 0)
		{
			if(pTemp->pLeft == NULL)
			{
				pTemp = GetPrev(pTemp);
				/*while( pTemp->SortComp(tempname) > 0)
				{
					pTemp = GetPrev(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}

			pTemp = pTemp->pLeft;
		}
		else
		{
			//pTemp = GetNext(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Hash<A>::Get(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		stepcount++;
		if(i < 0)//right
			pTemp = pTemp->pRight;
		else if(i > 0)
			pTemp = pTemp->pLeft;
		else
			return pTemp;
	}
	return NULL;
}

template <class A>
A* Hash<A>::GetNext(char* tempname, A *Start)
{
	int i;
	if(Start==NULL)
		return NULL;

	A* pTemp;

	pTemp = Start;
	i = pTemp->SortComp(tempname);
	if(i < 0)//right
		pTemp = pTemp->pRight;
	else if(i >= 0)
		pTemp = pTemp->pLeft;

	for(;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname);
		if(i < 0)//right
			pTemp = pTemp->pRight;
		else if(i > 0)
			pTemp = pTemp->pLeft;
		else
			return pTemp;
	}
	return NULL;
}

template <class A>
A* Hash<A>::Get(int &index,A *Start)
{
	if(Start==0)
		Start=this->pRoot;
	if(index==0)
		return Start;
	if(Start==0)
		return Start;
	A* tobj=NULL;
	index--;
	if(Start->pLeft!=NULL)
		tobj = Get(index, Start->pLeft);
	if(tobj==NULL && Start->pRight!=NULL)
		tobj = Get(index, Start->pRight);
	return tobj;
}

template <class A>
A* Hash<A>::OrderedGet(int &index,A *Start)
{
	if(Start==0)
		Start=this->pRoot;
	if(Start==0)
		return Start;

	A *tobj=NULL;
	if(Start->pLeft!=NULL)
		tobj = OrderedGet(index, Start->pLeft);
	if(tobj!=NULL)
		return tobj;

	if(index==0)
		return Start;

	index--;

	if(Start->pRight!=NULL)
		tobj = OrderedGet(index, Start->pRight);

	return tobj;
}

template <class A>
A* Hash<A>::CopyObj(A* obj)
{
	this->pTemp = new A(*obj);
	this->pTemp->pLeft = NULL;
	this->pTemp->pRight = NULL;
	this->pTemp->pParent = NULL;
	Create(this->pTemp);
	return this->pTemp;
}

template <class A>
char* Hash<A>::DisplayList()
{
	return NULL;
}

template <class A>
void Hash<A>::Sort()
{
}

template <class A>
void Hash<A>::RotLeft(A *Obj)
{
	//A *TempObj=NULL;
	if(Obj==NULL)
		return;
	if(Obj->pRight==NULL)
		return;

	A *pTemp;
	if(Obj->pParent!=NULL)
	{
		pTemp = Obj->pParent;
		if(pTemp->pLeft == Obj)
		{
			pTemp->pLeft = Obj->pRight;
		}
		else
		{
			pTemp->pRight = Obj->pRight;
		}
	}
	else
		this->pRoot = Obj->pRight;
	pTemp = Obj->pRight;
	Obj->pRight = pTemp->pLeft;
	if(Obj->pRight!=NULL)
		Obj->pRight->pParent = Obj;
	pTemp->pLeft = Obj;
	pTemp->pParent = Obj->pParent;
	Obj->pParent = pTemp;
}

template <class A>
void Hash<A>::RotRight(A *Obj)
{
	//A *TempObj=NULL;
	if(Obj==NULL)
		return;
	if(Obj->pLeft==NULL)
		return;

	A *pTemp;
	if(Obj->pParent!=NULL)
	{
		pTemp = Obj->pParent;
		if(pTemp->pLeft == Obj)
		{
			pTemp->pLeft = Obj->pLeft;
		}
		else
		{
			pTemp->pRight = Obj->pLeft;
		}
	}
	else
		this->pRoot = Obj->pLeft;
	pTemp = Obj->pLeft;
	Obj->pLeft = pTemp->pRight;
	if(Obj->pLeft!=NULL)
		Obj->pLeft->pParent = Obj;
	pTemp->pRight = Obj;
	pTemp->pParent = Obj->pParent;
	Obj->pParent = pTemp;
}


template <class A>
class IncrBinTree: public Hash<A>
{
public:
	virtual void	Create(A* currobj);
	virtual void	Clear(A *Start=0);

	RayRand32		rrand;
	unsigned int	randomness;//1 through 16
	int				autorandomness;

	int				TotalComp;
	unsigned int	Comps;

	IncrBinTree();
	virtual ~IncrBinTree() { Clear(); }
};

template <class A>
IncrBinTree<A>::IncrBinTree()
{
	this->iSorted = 1;
	this->pRoot = this->pTail = this->pTemp = 0;
	randomness = 2;
	autorandomness = 1;
	TotalComp = 0;
	Comps = 0;
}

template <class A>
void IncrBinTree<A>::Clear(A *Start)
{
	TotalComp = 0;
	Comps = 0;
	if(Start==0)
	{
		Start=this->pRoot;
		while(this->pRoot != NULL)
			delete this->pRoot;
		return;
	}

	if(Start==0)
		return;

	if(Start->pLeft!=0)
		Clear(Start->pLeft);
	if(Start->pRight!=0)
		Clear(Start->pRight);
	if(Start == this->pRoot)
		this->pRoot = NULL;
	if(Start == this->pTail)
		this->pTail = NULL;
	delete Start;
}

template <class A>
void IncrBinTree<A>::Create(A* currobj)
{
	if(currobj==NULL)
		return;

	this->Objects++;
	currobj->Cont = this;
	if(this->pRoot==NULL)
	{
		this->pRoot = this->pTail = currobj;
		return;
	}

	Comps++;
	TotalComp += this->pRoot->SortComp(currobj)<0 ? -1 : 1;

	if(autorandomness == 1)
	{
		randomness = -abs((int)(((double)TotalComp)/((double)Comps) * 15.0) ) + 16;
	}
	this->pTail = currobj;
	//if(rrand.iRand32() % 256 < 200)
	if(randomness > 16 || rrand.iRand32() % 1024 < (1018+randomness/3))
	{
		for(A *pTemp=this->pRoot; pTemp != NULL;)
		{
			if(pTemp->SortComp(currobj) < 0)//right
			{
				if(pTemp->pRight == NULL)
				{
					pTemp->pRight = currobj;
					currobj->pParent = pTemp;
					return;
				}
				pTemp = pTemp->pRight;
			}
			else//left
			{
				if(pTemp->pLeft == NULL)
				{
					pTemp->pLeft = currobj;
					currobj->pParent = pTemp;
					return;
				}
				pTemp = pTemp->pLeft;
			}
		}
	}
	else
	{
		unsigned int chance = 65536/log2(this->Objects);
		chance *= randomness;
		unsigned int a = 1;
		if(chance == 0)
			chance = 1;

		for(A *pTemp=this->pRoot; pTemp != NULL;)
		{
			if(chance != 0 && (a=rrand.iRand32()%65536) < chance)
			{
				if(rrand.iRand32() % 16 < 13 - randomness)
					//if(a & 3)
				{
					A *tobj2 = pTemp;
					int comp = currobj->SortComp(pTemp);

					if(comp < 0)
					{
						//currobj->pRight = pTemp;
						if(tobj2->pLeft != NULL)
						{
							//tobj2 = tobj2->pRight;
							while(tobj2->pLeft != NULL)
								tobj2 = tobj2->pLeft;

							Del(tobj2);
							this->Objects++;
						}
						else
							tobj2 = currobj;
					}
					else
					{
						//currobj->pLeft = pTemp;
						if(tobj2->pRight != NULL)
						{
							//tobj2 = tobj2->pLeft;
							while(tobj2->pRight != NULL)
								tobj2 = tobj2->pRight;

							Del(tobj2);
							this->Objects++;
						}
						else
							tobj2 = currobj;
					}

					if(pTemp->pParent != NULL)
					{
						if(pTemp->pParent->pLeft == pTemp)
						{
							pTemp->pParent->pLeft = tobj2;
						}
						else
						{
							pTemp->pParent->pRight = tobj2;
						}
					}
					tobj2->pParent = pTemp->pParent;
					pTemp->pParent = tobj2;
					if(pTemp == this->pRoot)
						this->pRoot = tobj2;

					if(comp < 0)
					{
						tobj2->pRight = pTemp;
					}
					else
					{
						tobj2->pLeft = pTemp;
					}
					//return;

					chance = 0;
					if(tobj2 == currobj)
						return;
					else
						pTemp = tobj2;
				}
				else
				{
					if(pTemp->pParent != NULL)
					{
						A *tobj2 = pTemp->pParent;
						chance = TotalComp/(Comps/32);
						if(chance < -30 || chance > 30)
						{
							if(TotalComp > 0)
								RotLeft( tobj2 );
							else
								RotRight( tobj2 );
						}
						else
						{
							if(a % 32 < 16)
								//if(TotalComp > 0)
								RotLeft( tobj2 );
							else
								RotRight( tobj2 );
						}
					}
					chance = 0;
				}
			}
			if(pTemp->SortComp(currobj) < 0)//right
			{
				if(pTemp->pRight == NULL)
				{
					pTemp->pRight = currobj;
					currobj->pParent = pTemp;
					return;
				}
				pTemp = pTemp->pRight;
			}
			else//left
			{
				if(pTemp->pLeft == NULL)
				{
					pTemp->pLeft = currobj;
					currobj->pParent = pTemp;
					return;
				}
				pTemp = pTemp->pLeft;
			}
		}
	}
}

template <class A>
class LinkedListIndex : public RayCont<A>
{
public:
	virtual void	Create(A* currobj);
	virtual void	Del(A* currobj);
	virtual A*		Get(char* tempname);
	virtual A*		Get(int &index, A *Start=0);
	virtual A*		GetNext(char* tempname, A *Start);
	virtual A*		OrderedGet(int &index, A *Start=0);
	virtual A*		CopyObj(A* obj);
	virtual char*	DisplayList();
	virtual void	Sort();
	virtual void	Clear(A *Start=0);

	int id;
	LinkedListIndex();
	LinkedListIndex(int Ind);
	virtual ~LinkedListIndex() { Clear();}
};

template <class A>
class Index : public RayCont<A>
{
public:
	virtual void	Create(A* currobj);
	virtual void	Del(A* currobj);
	virtual A*		Get(char* tempname);
	virtual A*		GetNext(char* tempname, A *Start);
	virtual A*		GetNext(A *Start);
	virtual A*		GetPrev(A *Start);
	virtual A*		Get(int &index, A *Start=0);
	virtual A*		OrderedGet(int &index, A *Start=0);
	virtual A*		CopyObj(A* obj);
	virtual char*	DisplayList();
	virtual void	Sort();
	virtual void	Clear(A *Start=0);
	virtual void	RotLeft(A *Obj);
	virtual void	RotRight(A *Obj);

	virtual A*		GetLessThan(char* tempname);
	virtual A*		GetGreaterThan(char* tempname);

	virtual A*		GetLessThanOrEqualTo(char* tempname);
	virtual A*		GetGreaterThanOrEqualTo(char* tempname);

	RayRand32		rrand;
	unsigned int	randomness;//1 through 16
	int				autorandomness;

	int				TotalComp;
	unsigned int	Comps;

	int id;//index id
	Index();
	Index(int iInd);
	virtual ~Index() { Clear(); }
};

template <class A>
Index<A>::Index(int iInd)
{
	id = iInd;
	this->iSorted = 1;
	this->pRoot = this->pTail = this->pTemp = 0;
	this->pParent = NULL;

	randomness = 2;
	autorandomness = 1;
	TotalComp = 0;
	Comps = 0;
}

template <class A>
void Index<A>::Clear(A *Start)
{
	if(Start==0)
	{
		Start=this->pRoot;
		while(this->pRoot != NULL)
			delete this->pRoot;
		return;
	}

	if(Start==0)
		return;

	if(Start->pLeft[id]!=0)
		Clear(Start->pLeft[id]);
	if(Start->pRight!=0)
		Clear(Start->pRight[id]);
	if(Start == this->pRoot)
		this->pRoot = NULL;
	if(Start == this->pTail)
		this->pTail = NULL;
	delete Start;
}

template <class A>
void Index<A>::Create(A* currobj)
{
	if(currobj==NULL)
		return;

	currobj->pParent[id] = currobj->pLeft[id] = currobj->pRight[id] = 0;

	this->Objects++;
	currobj->Cont = this->pParent;
	if(this->pRoot==NULL)
	{
		this->pRoot = this->pTail = currobj;
		return;
	}

	Comps++;
	TotalComp += this->pRoot->SortComp(currobj, id)<0 ? -1 : 1;

	if(autorandomness == 1)
	{
		randomness = -abs((int)(((double)TotalComp)/((double)Comps) * 15.0) ) + 16;
	}
	this->pTail = currobj;
	//if(rrand.iRand32() % 256 < 200)
	if(randomness > 16 || rrand.iRand32() % 1024 < (1018+randomness/3))
	{
		for(A *pTemp=this->pRoot; pTemp != NULL;)
		{
			if(pTemp->SortComp(currobj, id) < 0)//right
			{
				if(pTemp->pRight[id] == NULL)
				{
					pTemp->pRight[id] = currobj;
					currobj->pParent[id] = pTemp;
					return;
				}
				pTemp = pTemp->pRight[id];
			}
			else//left
			{
				if(pTemp->pLeft[id] == NULL)
				{
					pTemp->pLeft[id] = currobj;
					currobj->pParent[id] = pTemp;
					return;
				}
				pTemp = pTemp->pLeft[id];
			}
		}
	}
	else
	{
		unsigned int chance = 65536/log2(this->Objects);
		chance *= randomness;
		unsigned int a = 1;
		if(chance == 0)
			chance = 1;

		for(A *pTemp=this->pRoot; pTemp != NULL;)
		{
			if(chance != 0 && (a=rrand.iRand32()%65536) < chance)
			{
				if(rrand.iRand32() % 16 < 13 - randomness)
					//if(a & 3)
				{
					A *tobj2 = pTemp;
					int comp = currobj->SortComp(pTemp, id);

					if(comp < 0)
					{
						//currobj->pRight = pTemp;
						if(tobj2->pLeft[id] != NULL)
						{
							//tobj2 = tobj2->pRight;
							while(tobj2->pLeft[id] != NULL)
								tobj2 = tobj2->pLeft[id];

							Del(tobj2);
							this->Objects++;
						}
						else
							tobj2 = currobj;
					}
					else
					{
						//currobj->pLeft = pTemp;
						if(tobj2->pRight[id] != NULL)
						{
							//tobj2 = tobj2->pLeft;
							while(tobj2->pRight[id] != NULL)
								tobj2 = tobj2->pRight[id];

							Del(tobj2);
							this->Objects++;
						}
						else
							tobj2 = currobj;
					}

					if(pTemp->pParent[id] != NULL)
					{
						if(pTemp->pParent[id]->pLeft[id] == pTemp)
						{
							pTemp->pParent[id]->pLeft[id] = tobj2;
						}
						else
						{
							pTemp->pParent[id]->pRight[id] = tobj2;
						}
					}
					tobj2->pParent[id] = pTemp->pParent[id];
					pTemp->pParent[id] = tobj2;
					if(pTemp == this->pRoot)
						this->pRoot = tobj2;

					if(comp < 0)
					{
						tobj2->pRight[id] = pTemp;
					}
					else
					{
						tobj2->pLeft[id] = pTemp;
					}
					//return;

					chance = 0;
					if(tobj2 == currobj)
						return;
					else
						pTemp = tobj2;
				}
				else
				{
					if(pTemp->pParent[id] != NULL)
					{
						A *tobj2 = pTemp->pParent[id];
						chance = TotalComp/(Comps/32);
						if(chance < -30 || chance > 30)
						{
							if(TotalComp > 0)
								RotLeft( tobj2 );
							else
								RotRight( tobj2 );
						}
						else
						{
							if(a % 32 < 16)
								//if(TotalComp > 0)
								RotLeft( tobj2 );
							else
								RotRight( tobj2 );
						}
					}
					chance = 0;
				}
			}
			if(pTemp->SortComp(currobj, id) < 0)//right
			{
				if(pTemp->pRight[id] == NULL)
				{
					pTemp->pRight[id] = currobj;
					currobj->pParent[id] = pTemp;
					return;
				}
				pTemp = pTemp->pRight[id];
			}
			else//left
			{
				if(pTemp->pLeft[id] == NULL)
				{
					pTemp->pLeft[id] = currobj;
					currobj->pParent[id] = pTemp;
					return;
				}
				pTemp = pTemp->pLeft[id];
			}
		}
	}
}

template <class A>
void Index<A>::Del(A* currobj)
{
	if(currobj->pLeft[id] == NULL && currobj->pRight[id] == NULL)
	{
		if(this->pTemp == currobj)
			this->pTemp = currobj->pParent[id];
		if(this->pRoot == currobj)
			this->pRoot = NULL;
		if(this->pTail == currobj)
			this->pTail = currobj->pParent[id];
		if(currobj->pParent[id] != NULL)
		{
			if(currobj->pParent[id]->pLeft[id] == currobj)
				currobj->pParent[id]->pLeft[id] = NULL;
			else
				currobj->pParent[id]->pRight[id] = NULL;
		}
	}
	else if( currobj->pLeft[id] == NULL || currobj->pRight[id] == NULL)
	{
		if(currobj->pLeft[id] == NULL)
		{
			if(this->pTemp == currobj)
				this->pTemp = currobj->pRight[id];
			if(this->pRoot == currobj)
				this->pRoot = currobj->pRight[id];
			if(this->pTail == currobj)
				this->pTail = currobj->pRight[id];

			if(currobj->pParent[id] != NULL)
			{
				if(currobj->pParent[id]->pLeft[id] == currobj)
					currobj->pParent[id]->pLeft[id] = currobj->pRight[id];
				else
					currobj->pParent[id]->pRight[id] = currobj->pRight[id];
			}

			currobj->pRight[id]->pParent[id] = currobj->pParent[id];
		}
		else
		{
			if(this->pTemp == currobj)
				this->pTemp = currobj->pLeft[id];
			if(this->pRoot == currobj)
				this->pRoot = currobj->pLeft[id];
			if(this->pTail == currobj)
				this->pTail = currobj->pLeft[id];

			if(currobj->pParent[id] != NULL)
			{
				if(currobj->pParent[id]->pLeft[id] == currobj)
					currobj->pParent[id]->pLeft[id] = currobj->pLeft[id];
				else
					currobj->pParent[id]->pRight[id] = currobj->pLeft[id];
			}

			currobj->pLeft[id]->pParent[id] = currobj->pParent[id];
		}
	}
	else
	{
		A *TempItem;
		if(rand()&1)
		{
			TempItem = currobj->pLeft[id];
			while(TempItem->pRight[id] != NULL)
			{
				TempItem = TempItem->pRight[id];
			}
		}
		else
		{
			TempItem = currobj->pRight[id];
			while(TempItem->pLeft[id] != NULL)
			{
				TempItem = TempItem->pLeft[id];
			}
		}

		Del(TempItem);

		TempItem->pLeft[id] = currobj->pLeft[id];
		TempItem->pRight[id] = currobj->pRight[id];
		TempItem->pParent[id] = currobj->pParent[id];

		if(TempItem->pLeft[id]!=NULL)
			TempItem->pLeft[id]->pParent[id] = TempItem;
		if(TempItem->pRight[id]!=NULL)
			TempItem->pRight[id]->pParent[id] = TempItem;

		if(currobj->pParent[id] != NULL)
		{
			if(currobj->pParent[id]->pLeft[id] == currobj)
				currobj->pParent[id]->pLeft[id] = TempItem;
			else
				currobj->pParent[id]->pRight[id] = TempItem;
		}

		if(this->pTemp == currobj)
			this->pTemp = TempItem;
		if(this->pRoot == currobj)
			this->pRoot = TempItem;
		if(this->pTail == currobj)
			this->pTail = TempItem;

		return;
	}
	this->Objects--;
}

template <class A>
A* Index<A>::GetNext(A *Start)
{
	if(Start == NULL)
		return NULL;

	A *tobj = NULL;
	if(Start->pRight[id] != NULL)
	{
		tobj = Start->pRight[id];
		while(tobj->pLeft[id] != NULL)
			tobj = tobj->pLeft[id];
		return tobj;
	}

	if(Start->pParent[id] != NULL)
	{
		A *tobj2 = Start;
		tobj = tobj2->pParent[id];

		while( tobj2 != tobj->pLeft[id] )
		{
			tobj2 = tobj;
			tobj = tobj->pParent[id];
			if(tobj==NULL)
				return NULL;
		}

		return tobj;
	}
	return NULL;
}

template <class A>
A* Index<A>::GetPrev(A *Start)
{
	if(Start == NULL)
		return NULL;

	A *tobj = NULL;
	if(Start->pLeft[id] != NULL)
	{
		tobj = Start->pLeft[id];
		while(tobj->pRight[id] != NULL)
			tobj = tobj->pRight[id];
		return tobj;
	}

	if(Start->pParent[id] != NULL)
	{
		A *tobj2 = Start;
		tobj = tobj2->pParent[id];

		while( tobj != NULL && tobj2 != tobj->pRight[id] )
		{
			tobj2 = tobj;
			tobj = tobj->pParent[id];
			if(tobj==NULL)
				return NULL;
		}

		return tobj;
	}
	return NULL;
}

template <class A>
A* Index<A>::GetGreaterThan(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
		{
			if(pTemp->pRight[id] == NULL)
			{
				pTemp = GetNext(pTemp);
				/*while( pTemp->SortComp(tempname, id) < 0)
				{
					pTemp = GetNext(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}
			else
				pTemp = pTemp->pRight[id];
		}
		else if(i > 0)
		{
			if(pTemp->pLeft[id] == NULL)
				return pTemp;

			pTemp = pTemp->pLeft[id];
		}
		else
		{
			pTemp = GetPrev(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Index<A>::GetLessThan(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
		{
			if(pTemp->pRight[id] == NULL)
				return pTemp;
			else
				pTemp = pTemp->pRight[id];
		}
		else if(i > 0)
		{
			if(pTemp->pLeft[id] == NULL)
			{
				pTemp = GetPrev(pTemp);
				/*while( pTemp->SortComp(tempname, id) > 0)
				{
					pTemp = GetPrev(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}

			pTemp = pTemp->pLeft[id];
		}
		else
		{
			pTemp = GetNext(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Index<A>::GetGreaterThanOrEqualTo(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
		{
			if(pTemp->pRight[id] == NULL)
			{
				pTemp = GetNext(pTemp);
				/*while( pTemp->SortComp(tempname, id) < 0)
				{
					pTemp = GetNext(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}
			else
				pTemp = pTemp->pRight[id];
		}
		else if(i > 0)
		{
			if(pTemp->pLeft[id] == NULL)
				return pTemp;

			pTemp = pTemp->pLeft[id];
		}
		else
		{
			//pTemp = GetPrev(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Index<A>::GetLessThanOrEqualTo(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
		{
			if(pTemp->pRight[id] == NULL)
				return pTemp;
			else
				pTemp = pTemp->pRight[id];
		}
		else if(i > 0)
		{
			if(pTemp->pLeft[id] == NULL)
			{
				pTemp = GetPrev(pTemp);
				/*while( pTemp->SortComp(tempname, id) > 0)
				{
					pTemp = GetPrev(pTemp);
					cout << "crap\n";
				}*/
				return pTemp;
			}

			pTemp = pTemp->pLeft[id];
		}
		else
		{
			//pTemp = GetNext(pTemp);
			return pTemp;
		}
	}
	return NULL;
}

template <class A>
A* Index<A>::Get(char* tempname)
{
	int i;

	A* pTemp;

	//if(pTemp!=NULL && pTemp->SortComp(tempname) == 0)//not safe with multiple items with the same name
	//	return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
			pTemp = pTemp->pRight[id];
		else if(i > 0)
			pTemp = pTemp->pLeft[id];
		else
			return pTemp;
	}
	return NULL;
}

template <class A>
A* Index<A>::GetNext(char* tempname, A *Start)
{
	int i;
	if(Start==NULL)
		return NULL;

	A* pTemp;

	pTemp = Start;
	i = pTemp->SortComp(tempname, id);
	if(i < 0)//right
		pTemp = pTemp->pRight[id];
	else if(i >= 0)
		pTemp = pTemp->pLeft[id];

	for(;pTemp!=NULL;)
	{
		i = pTemp->SortComp(tempname, id);
		if(i < 0)//right
			pTemp = pTemp->pRight[id];
		else if(i > 0)
			pTemp = pTemp->pLeft[id];
		else
			return pTemp;
	}
	return NULL;
}

template <class A>
A* Index<A>::Get(int &index,A *Start)
{
	if(Start==0)
		Start=this->pRoot;
	if(index==0)
		return Start;
	if(Start==0)
		return Start;
	A* tobj=NULL;
	index--;
	if(Start->pLeft[id]!=NULL)
		tobj = Get(index, Start->pLeft[id]);
	if(tobj==NULL && Start->pRight[id]!=NULL)
		tobj = Get(index, Start->pRight[id]);
	return tobj;
}

template <class A>
A* Index<A>::OrderedGet(int &index,A *Start)
{
	if(Start==0)
		Start=this->pRoot;
	if(Start==0)
		return Start;

	A *tobj=NULL;
	if(Start->pLeft[id]!=NULL)
		tobj = OrderedGet(index, Start->pLeft[id]);
	if(tobj!=NULL)
		return tobj;

	if(index==0)
		return Start;

	index--;

	if(Start->pRight[id]!=NULL)
		tobj = OrderedGet(index, Start->pRight[id]);

	return tobj;
}

template <class A>
A* Index<A>::CopyObj(A* obj)
{
	this->pTemp = new A(*obj);
	this->pTemp->pLeft[id] = NULL;
	this->pTemp->pRight[id] = NULL;
	this->pTemp->pParent[id] = NULL;
	Create(this->pTemp);
	return this->pTemp;
}

template <class A>
char* Index<A>::DisplayList()
{
	return NULL;
}

template <class A>
void Index<A>::Sort()
{
}

template <class A>
void Index<A>::RotLeft(A *Obj)
{
	//A *TempObj=NULL;
	if(Obj==NULL)
		return;
	if(Obj->pRight[id]==NULL)
		return;

	A *pTemp;
	if(Obj->pParent[id]!=NULL)
	{
		pTemp = Obj->pParent[id];
		if(pTemp->pLeft[id] == Obj)
		{
			pTemp->pLeft[id] = Obj->pRight[id];
		}
		else
		{
			pTemp->pRight[id] = Obj->pRight[id];
		}
	}
	else
		this->pRoot = Obj->pRight[id];
	pTemp = Obj->pRight[id];
	Obj->pRight[id] = pTemp->pLeft[id];
	if(Obj->pRight[id]!=NULL)
		Obj->pRight[id]->pParent[id] = Obj;
	pTemp->pLeft[id] = Obj;
	pTemp->pParent[id] = Obj->pParent[id];
	Obj->pParent[id] = pTemp;
}

template <class A>
void Index<A>::RotRight(A *Obj)
{
	//A *TempObj=NULL;
	if(Obj==NULL)
		return;
	if(Obj->pLeft[id]==NULL)
		return;

	A *pTemp;
	if(Obj->pParent[id]!=NULL)
	{
		pTemp = Obj->pParent[id];
		if(pTemp->pLeft[id] == Obj)
		{
			pTemp->pLeft[id] = Obj->pLeft[id];
		}
		else
		{
			pTemp->pRight[id] = Obj->pLeft[id];
		}
	}
	else
		this->pRoot = Obj->pLeft[id];
	pTemp = Obj->pLeft[id];
	Obj->pLeft[id] = pTemp->pRight[id];
	if(Obj->pLeft[id]!=NULL)
		Obj->pLeft[id]->pParent[id] = Obj;
	pTemp->pRight[id] = Obj;
	pTemp->pParent[id] = Obj->pParent[id];
	Obj->pParent[id] = pTemp;
}


template <class A>
class MIndex : public RayCont<A>//MultiIndex
{
public:
	int iInds;
	RayCont<A> **Indecies;

	MIndex(int inds)
	{
		iInds = inds;
		if(iInds>0)//set an explicit 0 inds to make your own array for the Indexes
		{
			Indecies = new RayCont<A>*[iInds];
			for(int i=0;i<iInds;i++)
			{
				Indecies[i] = new Index<A>(i);
				Indecies[i]->pParent = this;
			}
		}
	}

	MIndex(int inds, int Types)
	{
		iInds = inds;
		Indecies = new RayCont<A>*[iInds];
		for(int i=0;i<iInds;i++)
		{
			if( Types & 1<<i )
				Indecies[i] = new LinkedListIndex<A>(i);
			else
				Indecies[i] = new Index<A>(i);
			Indecies[i]->pParent = this;
		}
	}

	virtual ~MIndex()
	{
		Clear();
		for(int i=0;i<iInds;i++)
		{
			delete Indecies[i];
		}
		delete[] Indecies;
	}

	virtual void Del(A *obj)
	{
		//if(Objects==890)
		//	cout<<"----890-----\n";

		for(int ind=0;ind<iInds;ind++)
			Indecies[ind]->Del(obj);
		//Indexes[0]->Del(obj);
		this->Objects--;
		//cout << "Objects == "<<Objects<<"\n";
	}

	virtual void Del(A *obj, int ind)
	{
		Indecies[ind]->Del(obj);
	}

	virtual void Clear(A *Start=0)
	{
		//for(int dim=0;dim<iDims;dim++)

		Indecies[0]->Clear();
		/*for(int i=0;i<iInds;i++)
		{
		while(Indexes[i]->pRoot!=NULL)
		Indexes[i]->Del(Indexes[i]->pRoot);
		cout << "Did "<<i<<"\n";
		}
		cout << "Done\n";*/

		//while(Indexes[0]->pRoot!=NULL)
		//	delete Indexes[0]->pRoot;
		/*int i = 0;
		Start = Get(i);
		while(Start!=NULL)
		{
		delete Start;
		i = 0;
		Start = Get(i);
		}*/
		this->Objects=0;
	}

	virtual A* CopyObj(A *obj)
	{
		return NULL;
	}

	virtual void Create(A* currobj)
	{
		currobj->Cont = this;
		for(int ind=0;ind<iInds;ind++)
			Indecies[ind]->Create(currobj);
		this->Objects++;
	}

	virtual void Create(A* currobj, int ind)
	{
		currobj->Cont = this;
		Indecies[ind]->Create(currobj);
	}

	virtual A* Get(char* tempname)
	{
		return Indecies[0]->Get(tempname);
	}

	virtual A* Get(int &index, A *Start)
	{
		return Indecies[0]->Get(index, Start);
	}

	virtual A* OrderedGet(int &index, A *Start)
	{
		return Indecies[0]->OrderedGet(index, Start);
	}

	virtual A* GetNext(char* tempname, A *Start)
	{
		return Indecies[0]->GetNext(tempname, Start);
	}

	virtual A* Get(char* tempname, int ind)
	{
		return Indecies[ind]->Get(tempname);
	}

	virtual A* Get(int &index, int ind)
	{
		return Indecies[ind]->Get(index, 0);
	}

	virtual A* OrderedGet(int &index, int ind)
	{
		return Indecies[ind]->OrderedGet(index, 0);
	}

	virtual A* GetNext(char* tempname, A *Start, int ind)
	{
		return Indecies[ind]->GetNext(tempname, Start);
	}

	virtual char* DisplayList()
	{
		return NULL;
	}

	virtual void Sort()
	{
	}
};



template <class A>
LinkedListIndex<A>::LinkedListIndex()
{
	this->iSorted = 0;
	this->pHead = this->pTail = this->pTemp = 0;
}

template <class A>
LinkedListIndex<A>::LinkedListIndex(int Ind)
{
	id = Ind;
	this->iSorted = 0;
	this->pHead = this->pTail = this->pTemp = 0;
}

template <class A>
void LinkedListIndex<A>::Clear(A *Start)
{
	while(this->pHead!=NULL)
		delete this->pHead;
}

template <class A>
void LinkedListIndex<A>::Create(A* currobj)
{
	if(currobj==NULL)
		return;
	this->Objects++;
	currobj->Cont = this->pParent;
	if(this->iSorted==0)
	{
		if(this->pHead==NULL)
			this->pHead = this->pTail = currobj;
		else
		{
			this->pTail->pNext[id] = currobj;
			currobj->pPrev[id] = this->pTail;
			this->pTail = currobj;
		}
	}
	else
	{
		A* pTemp;
		//add it with an insert sort, I thought about a quick sort, but it's a linked list, it would have to walk through every item anyways and in some cases even go back
		if(this->pHead==NULL)
		{
			this->pHead = this->pTail = currobj;
			return;
		}
		for(pTemp=this->pHead; pTemp != NULL; pTemp = pTemp->pNext[id])
		{
			if(pTemp->SortComp(currobj, id) > 0)//if currobj is supposed to come before pTemp
			{
				if(pTemp == this->pHead)
					this->pHead = currobj;
				if(pTemp->pPrev[id] != NULL)
					pTemp->pPrev[id]->pNext[id] = currobj;
				currobj->pPrev[id] = pTemp->pPrev[id];
				pTemp->pPrev[id] = currobj;
				currobj->pNext[id] = pTemp;
				return;
			}
		}
		this->pTail->pNext[id] = currobj;
		currobj->pPrev[id] = this->pTail;
		this->pTail = currobj;
	}
}

template <class A>
void LinkedListIndex<A>::Del(A* currobj)
{
	this->Objects--;
	if(currobj->pNext[id]!=NULL)
		currobj->pNext[id]->pPrev[id] = currobj->pPrev[id];
	if(currobj->pPrev[id]!=NULL)
		currobj->pPrev[id]->pNext[id] = currobj->pNext[id];
	if(this->pHead==currobj)
		this->pHead = currobj->pNext[id];
	if(this->pTail==currobj)
		this->pTail = currobj->pPrev[id];
}

template <class A>
A* LinkedListIndex<A>::Get(char* tempname)
{
	A* pTemp = this->pTemp;
	if(pTemp!=NULL && !pTemp->SortComp(tempname, id))
		return pTemp;

	for(pTemp=this->pHead;pTemp!=NULL;pTemp=pTemp->pNext[id])
		if(pTemp->name!=NULL && !pTemp->SortComp(tempname, id))
			return pTemp;
	return NULL;
}

template <class A>
A* LinkedListIndex<A>::GetNext(char* tempname, A* Start)
{
	if(Start==NULL)
		return NULL;
	for(A *pTemp=Start->pNext[id];pTemp!=NULL;pTemp=pTemp->pNext[id])
		if(pTemp->name!=NULL && !pTemp->SortComp(tempname, id))
			return pTemp;
	return NULL;
}

template <class A>
A* LinkedListIndex<A>::Get(int &index, A *Start)
{
	int i;
	int dista,distb;

	A* pTemp = this->pTemp;

	if(Start==0)
		Start=this->pHead;
	else
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext[id],i++)
			if(i==index)
				return pTemp;
		return NULL;
	}

	if(index>=this->Objects || index<0)
		return NULL;

	dista = index;
	distb = this->Objects - index;

	if(dista <= distb)
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext[id],i++)
			if(i==index)
				return pTemp;
	}
	else
	{
		for(pTemp=this->pTail, i=this->Objects-1;pTemp!=NULL;pTemp=pTemp->pPrev[id],i--)
			if(i==index)
				return pTemp;
	}
	return NULL;
}

template <class A>
A* LinkedListIndex<A>::OrderedGet(int &index, A *Start)
{
	int i;
	int dista,distb;

	A* pTemp = this->pTemp;

	if(Start==0)
		Start=this->pHead;
	else
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext[id],i++)
			if(i==index)
				return pTemp;
		return NULL;
	}

	if(index>=this->Objects || index<0)
		return NULL;

	dista = index;
	distb = this->Objects - index;

	if(dista <= distb)
	{
		for(pTemp=Start, i=0;pTemp!=NULL;pTemp=pTemp->pNext[id],i++)
			if(i==index)
				return pTemp;
	}
	else
	{
		for(pTemp=this->pTail, i=this->Objects-1;pTemp!=NULL;pTemp=pTemp->pPrev[id],i--)
			if(i==index)
				return pTemp;
	}
	return NULL;
}

template <class A>
A* LinkedListIndex<A>::CopyObj(A* obj)
{
	/*A* pTemp;
	pTemp = new A(*obj);
	pTemp->pNext = NULL;
	pTemp->pPrev = NULL;
	Create(pTemp);
	return pTemp;*/
	return NULL;
}

template <class A>
char* LinkedListIndex<A>::DisplayList()
{
	int i=0;
	A* pTemp = this->pTemp;
	for(pTemp=this->pHead;pTemp!=NULL;pTemp=pTemp->pNext[id])
	{
		i++;
		//cout << "("<<i<<") "<<pTemp->name<<"\n";
	}
	return 0;
	/*cout << "If you would like more info on something, type in it's name, otherwise just press Enter\n";
	cin >> tempstring;
	pTemp = Get(tempstring);
	if(pTemp !=NULL)
	{
	pTemp->MoreInfo();
	}
	cout<<"\n\n";*/
}

template <class A>
void LinkedListIndex<A>::Sort()
{
	//pTemp
	A* pTemp = this->pTemp;
	for(pTemp=this->pHead;pTemp!=NULL;)
	{
		if(pTemp->pNext[id] != NULL && pTemp->SortComp(pTemp->pNext[id], id)>0)
		{
			//switch the two in the linked list
			if(this->pHead == pTemp)
				this->pHead = pTemp->pNext[id];
			if(this->pTail == pTemp->pNext[id])
				this->pTail = pTemp;
			if(pTemp->pPrev[id]!=NULL)
				pTemp->pPrev[id]->pNext[id] = pTemp->pNext[id];
			if(pTemp->pNext[id]->pNext[id]!=NULL)
				pTemp->pNext[id]->pNext[id]->pPrev[id] = pTemp;
			pTemp->pNext[id]->pPrev[id] = pTemp->pPrev[id];
			pTemp->pPrev[id] = pTemp->pNext[id];
			pTemp->pNext[id] = pTemp->pPrev[id]->pNext[id];
			pTemp->pPrev[id]->pNext[id] = pTemp;

			pTemp = pTemp->pPrev[id]->pPrev[id];
			if(pTemp==NULL)
				pTemp = this->pHead;
		}
		else
			pTemp = pTemp->pNext[id];
	}
}

template<class A>
class SkipItem : public ContObj<SkipItem<A> >
{
public:
	A *Item;
	SkipItem<A> *pDown;
	SkipItem<A> *pUp;

	SkipItem()
	{
		pUp = NULL;
		pDown = NULL;
		Item = NULL;
	}
};

template<class A>
class MSkipItem : public A
{
public:
	MSkipItem<A> *pUp;

	MSkipItem()
	{
		pUp = NULL;
	}
};

template<class A>
class SkipItemList : public ContObj< SkipItemList<A> >
{
public:
	LinkedList<A> List;
};

template<class A>
class SkipList : public RayCont<A>
{
public:
	LinkedList< MSkipItem<A> > List;
	LinkedList< SkipItemList< SkipItem<A> > > SkipLists;

	LinkedList< SkipItemList< SkipItem<A> > > *TSList;

	SkipList()
	{
		this->iSorted = 1;
		List.iSorted = 1;
		TSList = NULL;
	}

	virtual ~SkipList()
	{
		while(List.pHead != NULL)
		{
			List.pHead->pUp = NULL;
			delete List.pHead;
		}
		List.~LinkedList< MSkipItem<A> >();
	}

	virtual void Create(A *currobj)
	{
		A *tobj = List.pHead;
		if(List.pHead == NULL)
		{
			List.pHead = List.pTail = currobj;
			List.Objects++;
			this->Objects++;
			return;
		}

	}

};

#include <buckettree.h>
