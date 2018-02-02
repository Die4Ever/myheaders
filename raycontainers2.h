#pragma once

/*TODO:
Thread Scalable containers
	-RTree
	-persistant hash table(locking on the outer hashes)
	-AVL Trees / Binary Trees?
		-this is tough...
		-maybe have read/write locks on the top 16 nodes or something...

fix GetAll function, doesn't work for AVLTrees
for the index caching on trees, shift items already in the cache on insertion and deletion
maybe on a cache miss check the other cache slots for the closest cached item?
index caching on linked lists
for Trees-
	-maybe add BeginsWith() searching
	-get less than or equal to
	-get greater than or equal to
	-get less than
	-get greater than

BucketTree
HashTable
SkipList?
Treap?
Heap?
Trie?
string searching with wildcards for BucketTree?

*/
#include <vector>
//using namespace std;
using std::vector;

class Cont2
{
public:

	virtual ~Cont2()
	{
	}
};

template <class A, class Key, unsigned int padding=0>
class LLObj
{
public:
	A			Obj;
	char		Padding[padding];
	LLObj<A,Key,padding>	*pNext;
	LLObj<A,Key,padding>	*pPrev;
	//Cont2		*pCont;

	LLObj()
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

	LLObj(A obj) : Obj(obj)
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

	LLObj(Key key) : Obj(key)
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

};

template <class A, class Key>
class LLObj<A,Key,0>
{
public:
	A			Obj;
	LLObj<A,Key,0>	*pNext;
	LLObj<A,Key,0>	*pPrev;
	//Cont2		*pCont;

	LLObj()
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

	LLObj(A obj) : Obj(obj)
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

	LLObj(Key key) : Obj(key)
	{
		pNext = pPrev = NULL;
		//pCont = NULL;
	}

};

//foreach iterator using the allocators arrays?
//sort function?
//sort by memory address?(but then I could just use the RayHeapList without a linked list, or a vector(swap an item to be deleted with the last item)...
//sorted flag?

template <class A, class Key, class Allocator, class LargeObj=A>
class LinkedList2 : public Cont2
{
public:
	typedef LLObj<A,Key, sizeof(LargeObj)-sizeof(A)> Node;

	Node			*pHead;
	Node			*pTail;
	unsigned int	Objects;

	Allocator	heap;

protected:
	void _Insert(Node * pobj)
	{
		Objects++;

		if(pHead != NULL)
		{
			pTail->pNext = pobj;
			pobj->pPrev = pTail;
			pTail = pobj;
		}
		else
			pHead = pTail = pobj;
	}

public:

	LinkedList2(int reserve=32) : heap(sizeof(Node), reserve)
	{
		pHead = pTail = NULL;
		Objects = 0;
	}

	LinkedList2(LinkedList2 &b) : heap(b.head)
	{
		pHead = b.pHead;
		pTail = b.pTail;
		b.pHead = b.pTail = NULL;
		Objects = b.Objects;
		b.Objects = 0;
	}

	~LinkedList2()
	{
	}

	template<class B>
	A *Create()
	{
		typedef LLObj<B,Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes(sizeof(Node));//RayNew(heap, (LLObj<A,Key>), () );
		new (pobj) NodeB();
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	template<class B>
	A *Create(B tobj)
	{
		typedef LLObj<B,Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		Node *pobj = (Node*)heap.AllocBytes(sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (tobj) );
		new (pobj) Node(tobj);
		_Insert((Node*)pobj);
		return (A*)pobj;
	}

	template<class B>
	A *CreateKey(const Key key)
	{
		typedef LLObj<B,Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		Node *pobj = (Node*)heap.AllocBytes(sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (key) );
		new (pobj) Node(key);
		_Insert((Node*)pobj);
		return (A*)pobj;
	}

	A *GetFirst()
	{
		return (A*)pHead;
	}

	A *GetLast()
	{
		return (A*)pTail;
	}

	A *GetNext(const A *obj)
	{
		return (A*)((Node*)obj)->pNext;
	}

	A *GetPrev(const A *obj)
	{
		return (A*)((Node*)obj)->pPrev;
	}

	A *Get(const Key key)
	{
		Node *obj = pHead;
		for( ;obj != NULL && obj->Obj.SortComp(key) != 0; obj=obj->pNext );

		return (A*)obj;
	}

	A *Index(unsigned int index)
	{
		unsigned int i;
		if(index>=Objects)
			return NULL;

		if(index <= Objects/2)
		{
			i=0;
			Node *p = pHead;
			while(i++<index)
				p=p->pNext;
			return (A*)p;
		}
		else
		{
			i=Objects-1;
			Node *p = pTail;
			while(i-->index)
				p=p->pPrev;
			return (A*)p;
		}
	}

	void Delete(A * obj)
	{
		Objects--;
		Node *pobj = (Node*)obj;
		if(pobj->pNext != NULL)
			pobj->pNext->pPrev = pobj->pPrev;
		if(pobj == pHead)
			pHead = pobj->pNext;
		if(pobj->pPrev != NULL)
			pobj->pPrev->pNext = pobj->pNext;
		if(pobj == pTail)
			pTail = pobj->pPrev;

		pobj->~Node();
		heap.Dealloc((char*)pobj);
	}

	void Clear()
	{
		while(pHead != NULL)
			Delete((A*)pHead);
	}
};

template <class A, class Key, unsigned int padding=0>
class BinTreeObj
{
public:
	A			Obj;
	char		Padding[padding];
	/*union
	{
		struct{*/
			BinTreeObj<A,Key,padding>	*pLeft;
			BinTreeObj<A,Key,padding>	*pRight;
		/*};
		BinTreeObj<A,Key,padding>	*pLR[2];
	};*/
	BinTreeObj<A,Key,padding>	*pParent;
	//Cont2		*pCont;

	BinTreeObj()
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

	BinTreeObj(A obj) : Obj(obj)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

	BinTreeObj(Key key) : Obj(key)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

};

template <class A, class Key>
class BinTreeObj<A,Key,0>
{
public:
	A			Obj;
	/*union
	{
		struct{*/
			BinTreeObj<A,Key,0>	*pLeft;
			BinTreeObj<A,Key,0>	*pRight;
		/*};
		BinTreeObj<A,Key,0>	*pLR[2];
	};*/
	BinTreeObj<A,Key,0>	*pParent;
	//Cont2		*pCont;

	BinTreeObj()
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

	BinTreeObj(A obj) : Obj(obj)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

	BinTreeObj(Key key) : Obj(key)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
	}

};

//get less than or equal to
//get greater than or equal to
//get less than
//get greater than
//get range
//get begins with?

template <class A, class Key, class Allocator, class LargeObj=A, unsigned int CACHES=1>
class BinaryTree2 : public Cont2
{
public:
	typedef BinTreeObj<A,Key, sizeof(LargeObj)-sizeof(A)> Node;

	Node			*pRoot;
	unsigned int	Objects;

	Allocator	heap;

	unsigned int	Indecies[CACHES];
	Node			*IndexedNodes[CACHES];

protected:
	void _Insert(Node * pobj)
	{
		if(CACHES>1)
			memset(IndexedNodes, 0, sizeof(Node*)*CACHES);
		Objects++;

		if(pRoot != NULL)
		{
			for(Node *p=pRoot; ;)
			{
				if(p->Obj.SortComp((A*)pobj) >= 0)//left
				{
					if(p->pLeft != NULL)
						p=p->pLeft;
					else
					{
						p->pLeft = pobj;
						pobj->pParent = p;
						return;
					}
				}
				else//right
				{
					if(p->pRight != NULL)
						p=p->pRight;
					else
					{
						p->pRight = pobj;
						pobj->pParent = p;
						return;
					}
				}
			}
		}
		else
		{
			pRoot = pobj;
		}
	}

	void _Unindex(Node *pobj)
	{

		if(pobj->pLeft == NULL && pobj->pRight == NULL)
		{
			if(pRoot == pobj)
				pRoot = NULL;
			if(pobj->pParent != NULL)
			{
				if(pobj->pParent->pLeft == pobj)
					pobj->pParent->pLeft = NULL;
				else
					pobj->pParent->pRight = NULL;
			}
		}
		else if( pobj->pLeft == NULL || pobj->pRight == NULL)
		{
			if(pobj->pLeft == NULL)
			{
				if(pRoot == pobj)
					pRoot = pobj->pRight;

				if(pobj->pParent != NULL)
				{
					if(pobj->pParent->pLeft == pobj)
						pobj->pParent->pLeft = pobj->pRight;
					else
						pobj->pParent->pRight = pobj->pRight;
				}

				pobj->pRight->pParent = pobj->pParent;
			}
			else
			{
				if(pRoot == pobj)
					pRoot = pobj->pLeft;

				if(pobj->pParent != NULL)
				{
					if(pobj->pParent->pLeft == pobj)
						pobj->pParent->pLeft = pobj->pLeft;
					else
						pobj->pParent->pRight = pobj->pLeft;
				}

				pobj->pLeft->pParent = pobj->pParent;
			}
		}
		else
		{
			Node *TempItem;
			if(rand()&1)
			{
				TempItem = pobj->pLeft;
				while(TempItem->pRight != NULL)
				{
					TempItem = TempItem->pRight;
				}
			}
			else
			{
				TempItem = pobj->pRight;
				while(TempItem->pLeft != NULL)
				{
					TempItem = TempItem->pLeft;
				}
			}

			_Unindex(TempItem);

			TempItem->pLeft = pobj->pLeft;
			TempItem->pRight = pobj->pRight;
			TempItem->pParent = pobj->pParent;

			if(TempItem->pLeft!=NULL)
				TempItem->pLeft->pParent = TempItem;
			if(TempItem->pRight!=NULL)
				TempItem->pRight->pParent = TempItem;

			if(pobj->pParent != NULL)
			{
				if(pobj->pParent->pLeft == pobj)
					pobj->pParent->pLeft = TempItem;
				else
					pobj->pParent->pRight = TempItem;
			}

			if(pRoot == pobj)
				pRoot = TempItem;

			return;
		}
		Objects--;
		if(CACHES>1)
			memset(IndexedNodes, 0, sizeof(Node*)*CACHES);
	}

	void _RotLeft(Node *pobj)
	{
		if(pobj->pRight==NULL)
			return;

		Node *pTemp;
		if(pobj->pParent!=NULL)
		{
			pTemp = pobj->pParent;
			if(pTemp->pLeft == pobj)
				pTemp->pLeft = pobj->pRight;
			else
				pTemp->pRight = pobj->pRight;
		}
		else
			pRoot = pobj->pRight;

		pTemp = pobj->pRight;
		pobj->pRight = pTemp->pLeft;
		if(pobj->pRight!=NULL)
			pobj->pRight->pParent = pobj;
		pTemp->pLeft = pobj;
		pTemp->pParent = pobj->pParent;
		pobj->pParent = pTemp;
	}

	void _RotRight(Node *pobj)
	{
		if(pobj->pLeft==NULL)
			return;

		Node *pTemp;
		if(pobj->pParent!=NULL)
		{
			pTemp = pobj->pParent;
			if(pTemp->pLeft == pobj)
				pTemp->pLeft = pobj->pLeft;
			else
				pTemp->pRight = pobj->pLeft;
		}
		else
			pRoot = pobj->pLeft;

		pTemp = pobj->pLeft;
		pobj->pLeft = pTemp->pRight;
		if(pobj->pLeft!=NULL)
			pobj->pLeft->pParent = pobj;
		pTemp->pRight = pobj;
		pTemp->pParent = pobj->pParent;
		pobj->pParent = pTemp;
	}

	BinaryTree2(unsigned int nodesize, int reserve) : heap(nodesize, reserve)
	{
		if(CACHES>1)
			memset(IndexedNodes, 0, sizeof(Node*)*CACHES);

		pRoot = NULL;
		Objects = 0;
	}

public:

	BinaryTree2(int reserve) : heap((unsigned int)sizeof(Node), reserve)
	{
		if(CACHES>1)
			memset(IndexedNodes, 0, sizeof(Node*)*CACHES);

		pRoot = NULL;
		Objects = 0;
	}

	~BinaryTree2()
	{
	}

	/*template <class B>
	A *Create()
	{
		typedef BinTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), () );
		new (pobj) NodeB();
		_Insert((Node*)pobj);
		return (B*)pobj;
	}*/

	template <class B>
	A *Create(B tobj)
	{
		typedef BinTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (tobj) );
		new (pobj) NodeB(tobj);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	template <class B>
	A *CreateKey(const Key key)
	{
		typedef BinTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (key) );
		new (pobj) NodeB(key);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	A *GetFirst()
	{
		Node *pobj = pRoot;
		if(pobj == NULL)
			return NULL;

		while(pobj->pLeft!=NULL)
			pobj = pobj->pLeft;
		return (A*)pobj;
	}

	A *GetLast()
	{
		Node *pobj = pRoot;
		if(pobj == NULL)
			return NULL;

		while(pobj->pRight!=NULL)
			pobj = pobj->pRight;
		return (A*)pobj;
	}

	A *GetFirst(const Key key)
	{
		Node *p = (Node*)Get(key);
		Node *p2;

		if(p==NULL)
			return NULL;

		while(1)
		{
			p2 = (Node*)GetPrev((A*)p);
			if( p2==NULL || p2->Obj.SortComp(key) != 0 )
				break;
			p=p2;
			p2=p2->pLeft;
			for(; p2!=NULL && p2->Obj.SortComp(key) == 0; p2 = p2->pLeft )
				p=p2;
		}

		return (A*)p;
	}

	A *GetLast(const Key key)
	{
		Node *p = (Node*)Get(key);
		Node *p2;
		
		if(p==NULL)
			return NULL;

		while(1)
		{
			p2 = (Node*)GetNext((A*)p);
			if( p2==NULL || p2->Obj.SortComp(key) != 0 )
				break;
			p=p2;
			p2=p2->pRight;
			for(; p2!=NULL && p2->Obj.SortComp(key) == 0; p2 = p2->pRight )
				p=p2;
		}

		return (A*)p;
	}

	A *GetNext(const A *obj)
	{
		Node *Start = (Node *)obj;
		Node *tobj = NULL;
		if(Start->pRight != NULL)
		{
			tobj = Start->pRight;
			while(tobj->pLeft != NULL)
				tobj = tobj->pLeft;
			return (A*)tobj;
		}

		if(Start->pParent != NULL)
		{
			Node *tobj2 = Start;
			tobj = tobj2->pParent;

			while( tobj2 != tobj->pLeft )
			{
				tobj2 = tobj;
				tobj = tobj->pParent;
				if(tobj==NULL)
					return NULL;
			}

			return (A*)tobj;
		}
		return NULL;
	}

	A *GetPrev(const A *obj)
	{
		Node *Start = (Node *)obj;
		Node *tobj = NULL;
		if(Start->pLeft != NULL)
		{
			tobj = Start->pLeft;
			while(tobj->pRight != NULL)
				tobj = tobj->pRight;
			return (A*)tobj;
		}

		if(Start->pParent != NULL)
		{
			Node *tobj2 = Start;
			tobj = tobj2->pParent;

			while( tobj != NULL && tobj2 != tobj->pRight )
			{
				tobj2 = tobj;
				tobj = tobj->pParent;
				if(tobj==NULL)
					return NULL;
			}

			return (A*)tobj;
		}
		return NULL;
	}

	A *Get(const Key key)
	{
		for(Node *p=pRoot;p!=NULL;)
		{
			int i = p->Obj.SortComp(key);
			if(i > 0)//left
			{
				p=p->pLeft;
			}
			else if(i < 0)
			{
				p=p->pRight;
			}
			else
				return (A*)p;
		}

		return NULL;
	}

	A *Get(A *Start, const Key key)
	{
		for(Node *p=(Node*)Start;p!=NULL;)
		{
			int i = p->Obj.SortComp(key);
			if(i > 0)//left
			{
				p=p->pLeft;
			}
			else if(i < 0)
			{
				p=p->pRight;
			}
			else
				return (A*)p;
		}
		return NULL;
	}

	void GetAll(const Key key, vector<A*> &ret)
	{
		Node *p1 = (Node*)GetFirst(key);
		Node *p2 = (Node*)GetLast(key);
		for(Node *p=p1; p!=NULL && p!=p2; p=(Node*)GetNext((A*)p))
		{
			ret.push_back((A*)p);
		}
		if(p2 != NULL)
			ret.push_back((A*)p2);
	}

	A *Index(unsigned int index)
	{
		unsigned int i;
		Node *p;
		if(index >= Objects)
			return NULL;

		unsigned int slot;
		if(CACHES>1)
		{
			slot = index/((Objects+CACHES)/CACHES);
			if(IndexedNodes[slot] != NULL)
			{
				i=Indecies[slot];
				p=IndexedNodes[slot];
				if(i<=index)
				{
					while(i++<index)
						p=(Node*)GetNext((A*)p);
				}
				else
				{
					while(i-->index)
						p=(Node*)GetPrev((A*)p);
				}

				return (A*)p;
			}
		}

		if(index <= Objects/2)
		{
			i=0;
			p=(Node*)GetFirst();
			while(i++<index)
				p=(Node*)GetNext((A*)p);
		}
		else
		{
			i=Objects-1;
			p=(Node*)GetLast();
			while(i-->index)
				p=(Node*)GetPrev((A*)p);
		}

		if(CACHES>1)
		{
			IndexedNodes[slot] = p;
			Indecies[slot] = index;
		}

		return (A*)p;
	}

	void Delete(A *obj)
	{
		Node *pobj = (Node*)obj;
		_Unindex(pobj);
		pobj->~Node();
		heap.Dealloc((char*)pobj);
	}

	void Clear()
	{
		while(pRoot != NULL)
			Delete((A*)pRoot);
	}

	void Reindex(A *obj)
	{
		Node *pobj = (Node*)obj;
		_Unindex(pobj);
		_Insert(pobj);
	}
};

template<class A, class Key, unsigned int padding=0>
class AVLTreeObj
{
public:
	A			Obj;
	char		Padding[padding];

	/*union
	{
		struct{*/
			AVLTreeObj<A,Key,padding>	*pLeft;
			AVLTreeObj<A,Key,padding>	*pRight;
		/*};
		AVLTreeObj<A,Key,padding>	*pLR[2];
	};*/
	AVLTreeObj<A,Key,padding>	*pParent;
	//Cont2				*pCont;

	int LObjects;
	int RObjects;

	AVLTreeObj()
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	AVLTreeObj(A obj) : Obj(obj)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	AVLTreeObj(Key key) : Obj(key)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	int CalcObjects()
	{
		int tLObjects=0;
		int tRObjects=0;
		if(pLeft!=NULL)
		{
			if(pLeft->LObjects > pLeft->RObjects)
				tLObjects=pLeft->LObjects+1;
			else
				tLObjects=pLeft->RObjects+1;
		}
		if(pRight!=NULL)
		{
			if(pRight->LObjects > pRight->RObjects)
				tRObjects=pRight->LObjects+1;
			else
				tRObjects=pRight->RObjects+1;
		}

		if(tLObjects == LObjects && tRObjects == RObjects)
			return 0;
		if( max(tLObjects,tRObjects) == max(LObjects,RObjects) )
		{
			LObjects = tLObjects;
			RObjects = tRObjects;
			return 0;
		}
		LObjects = tLObjects;
		RObjects = tRObjects;
		return 1;
	}

};

template<class A, class Key>
class AVLTreeObj<A,Key,0>
{
public:
	A			Obj;
	/*union
	{
		struct{*/
			AVLTreeObj<A,Key,0>	*pLeft;
			AVLTreeObj<A,Key,0>	*pRight;
		/*};
		AVLTreeObj<A,Key,0>	*pLR[2];
	};*/
	AVLTreeObj<A,Key,0>	*pParent;
	//Cont2				*pCont;

	int LObjects;
	int RObjects;

	AVLTreeObj()
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	AVLTreeObj(A obj) : Obj(obj)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	AVLTreeObj(Key key) : Obj(key)
	{
		pParent = pLeft = pRight = NULL;
		//pCont = NULL;
		LObjects = RObjects = 0;
	}

	int CalcObjects()
	{
		int tLObjects=0;
		int tRObjects=0;
		if(pLeft!=NULL)
		{
			if(pLeft->LObjects > pLeft->RObjects)
				tLObjects=pLeft->LObjects+1;
			else
				tLObjects=pLeft->RObjects+1;
		}
		if(pRight!=NULL)
		{
			if(pRight->LObjects > pRight->RObjects)
				tRObjects=pRight->LObjects+1;
			else
				tRObjects=pRight->RObjects+1;
		}

		if(tLObjects == LObjects && tRObjects == RObjects)
			return 0;
		if( max(tLObjects,tRObjects) == max(LObjects,RObjects) )
		{
			LObjects = tLObjects;
			RObjects = tRObjects;
			return 0;
		}
		LObjects = tLObjects;
		RObjects = tRObjects;
		return 1;
	}

};

template <class A, class Key, class Allocator, class LargeObj=A, unsigned int CACHES=1>
class AVLTree2 : public BinaryTree2<A,Key,Allocator, LargeObj, CACHES>
{
public:
	typedef AVLTreeObj<A,Key, sizeof(LargeObj)-sizeof(A)> Node;
	typedef BinTreeObj<A,Key, sizeof(LargeObj)-sizeof(A)> BNode;

protected:
	void RotLeft(Node *Obj)
	{
		_RotLeft((BNode*)Obj);
		Obj->CalcObjects();
		if(Obj->pParent!=NULL)
			Obj->pParent->CalcObjects();
	}

	void RotRight(Node *Obj)
	{
		_RotRight((BNode*)Obj);
		Obj->CalcObjects();
		if(Obj->pParent!=NULL)
			Obj->pParent->CalcObjects();
	}

	void Balance(Node *pTemp)
	{
		for(;pTemp!=NULL;pTemp=pTemp->pParent)
		{
			if(pTemp->CalcObjects())
			{
				if( abs(pTemp->LObjects - pTemp->RObjects) > 1 )
				{
					if(pTemp->LObjects > pTemp->RObjects)
					{
						RotRight(pTemp);
						pTemp=pTemp->pParent;
					}
					else
					{
						RotLeft(pTemp);
						pTemp=pTemp->pParent;
					}
				}
			}
			else
				break;
		}
	}

	void _Insert(Node *obj)
	{
		BinaryTree2<A,Key,Allocator,LargeObj,CACHES>::_Insert((BNode*)obj);
		obj->CalcObjects();
		Balance(obj->pParent);
	}

	void _Unindex(Node *pobj)
	{
		if(pobj->pLeft == NULL && pobj->pRight == NULL)
		{
			if((Node*)this->pRoot == pobj)
				this->pRoot = NULL;
			if(pobj->pParent != NULL)
			{
				if(pobj->pParent->pLeft == pobj)
					pobj->pParent->pLeft = NULL;
				else
					pobj->pParent->pRight = NULL;

				Balance(pobj->pParent);
			}
		}
		else if( pobj->pLeft == NULL || pobj->pRight == NULL)
		{
			if(pobj->pLeft == NULL)
			{
				if((Node*)this->pRoot == pobj)
					this->pRoot = (BNode*)pobj->pRight;

				if(pobj->pParent != NULL)
				{
					if(pobj->pParent->pLeft == pobj)
						pobj->pParent->pLeft = pobj->pRight;
					else
						pobj->pParent->pRight = pobj->pRight;

					pobj->pRight->pParent = pobj->pParent;
					Balance(pobj->pParent);
				}
				else
					pobj->pRight->pParent = pobj->pParent;
			}
			else
			{
				if((Node*)this->pRoot == pobj)
					this->pRoot = (BNode*)pobj->pLeft;

				if(pobj->pParent != NULL)
				{
					if(pobj->pParent->pLeft == pobj)
						pobj->pParent->pLeft = pobj->pLeft;
					else
						pobj->pParent->pRight = pobj->pLeft;

					pobj->pLeft->pParent = pobj->pParent;
					Balance(pobj->pParent);
				}
				else
					pobj->pLeft->pParent = pobj->pParent;
			}
		}
		else
		{
			Node *TempItem;
			if(rand()&1)
			{
				TempItem = pobj->pLeft;
				while(TempItem->pRight != NULL)
				{
					TempItem = TempItem->pRight;
				}
			}
			else
			{
				TempItem = pobj->pRight;
				while(TempItem->pLeft != NULL)
				{
					TempItem = TempItem->pLeft;
				}
			}

			_Unindex(TempItem);

			TempItem->pLeft = pobj->pLeft;
			TempItem->pRight = pobj->pRight;
			TempItem->pParent = pobj->pParent;

			if(TempItem->pLeft!=NULL)
				TempItem->pLeft->pParent = TempItem;
			if(TempItem->pRight!=NULL)
				TempItem->pRight->pParent = TempItem;

			if(pobj->pParent != NULL)
			{
				if(pobj->pParent->pLeft == pobj)
					pobj->pParent->pLeft = TempItem;
				else
					pobj->pParent->pRight = TempItem;
			}

			if((Node*)this->pRoot == pobj)
				this->pRoot = (BNode*)TempItem;

			return;
		}
		this->Objects--;
		if(CACHES>1)
			memset(this->IndexedNodes, 0, sizeof(Node*)*CACHES);
	}

public:
	AVLTree2(int reserve=128) : BinaryTree2<A,Key,Allocator,LargeObj,CACHES>((unsigned int)sizeof(Node), reserve)
	{
	}

	/*template <class B>
	A *Create()
	{
		typedef AVLTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT(sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), () );
		new (pobj) NodeB();
		_Insert((Node*)pobj);
		return (B*)pobj;
	}*/

	template <class B>
	A *CreateKey(const Key key)
	{
		typedef AVLTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT(sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)this->heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(this->heap, (LLObj<A,Key>), (key) );
		new (pobj) NodeB(key);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	template <class B>
	A *Create(B tobj)
	{
		typedef AVLTreeObj<B, Key, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT(sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)this->heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(this->heap, (LLObj<A,Key>), (tobj) );
		new (pobj) NodeB(tobj);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	void Delete(A *obj)
	{
		Node *pobj = (Node*)obj;
		_Unindex(pobj);
		pobj->~Node();
		this->heap.Dealloc((char*)pobj);
	}

	void Clear()
	{
		while(this->pRoot != NULL)
			Delete((A*)this->pRoot);
	}

	void Reindex(A *obj)
	{
		Node *pobj = (Node*)obj;
		_Unindex(pobj);
		_Insert(pobj);
	}

};

class Basic_Cont_Object_String
{
public:
	char *name;

	int SortComp(Basic_Cont_Object_String* objB)
	{
		if(name==NULL)
			return -1;
		if(objB->name == NULL)
			return 1;
		return strcmp(name, objB->name);
	}

	int SortComp(char *objB)
	{
		if(name==NULL)
			return -1;
		if(objB == NULL)
			return 1;
		return strcmp(name, objB);
	}

	Basic_Cont_Object_String()
	{
		name = NULL;
	}

	virtual ~Basic_Cont_Object_String()
	{
		delete[] name;
	}
};

class Basic_Cont_Object_Int
{
public:
	int ID;

	//watch for integer overflows!
	int SortComp(Basic_Cont_Object_Int *objB)
	{
		return ID - objB->ID;
	}

	int SortComp(int objB)
	{
		return ID - objB;
	}
	//alternative-
	/*
	int SortComp(Basic_Cont_Object_Int *objB)
	{
		if( ID < objB->ID )
			return -1;
		else if( ID > objB->ID )
			return 1;
		return 0;
	}

	int SortComp(int objB)
	{
		if( ID < objB )
			return -1;
		else if( ID > objB )
			return 1;
		return 0;
	}
	*/

	Basic_Cont_Object_Int()
	{
		ID = 0;
	}

	virtual ~Basic_Cont_Object_Int()
	{
	}
};
