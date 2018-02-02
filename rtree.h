#pragma once

/*TODO
	-items should be heap sorted by size
		-when adding an item, if it is larger than the current item being checked against, then swap them
		-I should maintain the heap when doing the _UnindexChildren function

	-how to make a thread scalable RTree?
		-divide world into sections
			-template the size of each tile (like 1024x1024 units for each tile or something)
			-use a hash table for the tiles (if the table was 16 slots, tile 32 would go in slot 0)
			-when inserting, go by the center point of the object
			-read/write locks?
			-when searching, lock on all tiles the search space intersects
			-when locking, lock the tiles in numerical order to prevent deadlocks(this fix should come naturally anyways)
*/

template <class A, class Key, unsigned int Children, unsigned int padding=0>
class RTreeObj
{
public:
	A			Obj;
	char		Padding[padding];
	RTreeObj<A,Key,Children,padding>	*pChildren[Children];

	RTreeObj<A,Key,Children,padding>	*pParent;
	//Cont2		*pCont;

	RTreeObj()
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

	RTreeObj(A obj) : Obj(obj)
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

	RTreeObj(Key key) : Obj(key)
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

};

template <class A, class Key, unsigned int Children>
class RTreeObj<A,Key,Children,0>
{
public:
	A			Obj;
	RTreeObj<A,Key,Children,0>	*pChildren[Children];

	RTreeObj<A,Key,Children,0>	*pParent;
	//Cont2		*pCont;

	RTreeObj()
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

	RTreeObj(A obj) : Obj(obj)
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

	RTreeObj(Key key) : Obj(key)
	{
		pParent = NULL;
		memset(pChildren, 0, sizeof(pChildren));
		//pCont = NULL;
	}

};

template <class A, class Key, class Allocator, unsigned int Children=4, class LargeObj=A>
class RTree : public Cont2
{
public:
	typedef RTreeObj<A,Key,Children,sizeof(LargeObj)-sizeof(A)> Node;
	//typedef RTreeObj<LargeObj,Key,Children> LargeNode;

	Node			*pRoot;
	unsigned int	Objects;
	//unsigned int	MaxHeight;

	Allocator	heap;

protected:

	void _AdjustSwap(Node * pobj)
	{
		for(int i=0; i<Children; i++)
		{
			if(pobj->pChildren[i] != NULL && pobj->pChildren[i]->Obj.area > pobj->Obj.area)
			{
				Node *p = pobj->pChildren[i];
				Node *ps[Children];

				p->pParent = pobj->pParent;
				if(p->pParent == NULL)
					pRoot = p;

				for(unsigned int c=0; c<Children; c++)
				{
					if(p->pParent != NULL && p->pParent->pChildren[c] == pobj)
						p->pParent->pChildren[c] = p;
					ps[c] = pobj->pChildren[c];
					pobj->pChildren[c] = p->pChildren[c];

					if(pobj->pChildren[c] != NULL)
						pobj->pChildren[c]->pParent = pobj;

					p->pChildren[c] = ps[c];
					if(p->pChildren[c] == p)
						p->pChildren[c] = pobj;
					if(p->pChildren[c] != NULL)
						p->pChildren[c]->pParent = p;
				}
				i=-1;
				_AdjustSwap(p);
			}
		}

		SmallMove((A*)pobj);//I'm really not sure about this...might have to do it more often...
	}

	void _Insert(Node * pobj)
	{
		Objects++;
		//pobj->pCont = this;

		//unsigned int height = 0;

		if(pRoot != NULL)
		{
			unsigned int child = 0;
			//unsigned int cost = 0;

			//pRoot->Obj.ResizeTo(&pobj->Obj);

			for(Node *p=pRoot; ;/*height++*/)
			{
				if(pobj->Obj.area > p->Obj.area)
				{
					pobj->pParent = p->pParent;
					for(unsigned int i=0;i<Children;i++)
					{
						if(p->pParent != NULL)
							if(p->pParent->pChildren[i] == p)
								p->pParent->pChildren[i] = pobj;

						pobj->pChildren[i] = p->pChildren[i];
						if(p->pChildren[i] != NULL)
							p->pChildren[i]->pParent = pobj;
						p->pChildren[i] = NULL;
					}
					p->pParent=NULL;

					SmallMove( (A*)pobj );
					if(pobj->pParent != NULL)
						SmallMove( (A*)pobj->pParent );
					else
						pRoot = pobj;

					p->Obj.ShrinkRegion();
					Node *t = pobj;
					pobj = p;
					p = t;
				}
				p->Obj.ResizeTo(&pobj->Obj);
				child = p->Obj.GetChild(&pobj->Obj);
				if(p->pChildren[child]!=NULL)
				{
					p=p->pChildren[child];
				}
				else
				{
					p->pChildren[child] = pobj;
					pobj->pParent = p;
					break;
				}
			}
		}
		else
		{
			pRoot = pobj;
		}

		//if(height>MaxHeight)
		//	MaxHeight=height;
	}

	void _UnindexNoChildren(Node *pobj)
	{
		if(pobj->pParent != NULL)
		{
			for(unsigned int c=0;c<Children;c++)
			{
				if(pobj->pParent->pChildren[c] == pobj)
				{
					pobj->pParent->pChildren[c] = NULL;
					break;
				}
			}
		}
		else
			pRoot = NULL;

		pobj->pParent = NULL;
	}

	void _Unindex1Child(Node *pobj)
	{
		unsigned int c = 0;
		for(;c<Children;c++)
		{
			if(pobj->pChildren[c] != NULL)
				break;
		}

		pobj->pChildren[c]->pParent = pobj->pParent;

		if(pobj->pParent != NULL)
		{
			for(unsigned int i=0;i<Children;i++)
			{
				if(pobj->pParent->pChildren[i] == pobj)
				{
					pobj->pParent->pChildren[i] = pobj->pChildren[c];
					break;
				}
			}
		}
		else
			pRoot = pobj->pChildren[c];

		pobj->pChildren[c] = NULL;
		pobj->pParent = NULL;
	}

	void _UnindexChildren(Node *pobj)
	{
		//pobj->Obj.FillRegion();
		unsigned int child = 0;
		unsigned int cost = 0;

		if(Objects==0)
		{
			pRoot = NULL;
			return;
		}
		for(Node *p = pobj; ;)
		{
			child=0;
			if(p->pChildren[0] != NULL)
				cost=p->pChildren[0]->Obj.CostResizeToRegion(&pobj->Obj);
			else
				cost = ~0;

			for(unsigned int c=1;c<Children;c++)
			{
				if(p->pChildren[c] != NULL)
				{
					unsigned int newcost = p->pChildren[c]->Obj.CostResizeToRegion(&pobj->Obj);
					if(newcost<cost)
					{
						cost = newcost;
						child = c;
					}
				}
			}
			if(p->pChildren[child]!=NULL)
			{
				p=p->pChildren[child];
			}
			else
			{
				Node *o = p->pParent;
				_UnindexNoChildren(p);
				unsigned int c;

				if(pobj->pParent!=NULL)
				{
					for(c=0;c<Children;c++)
					{
						if(pobj->pParent->pChildren[c] == pobj)
						{
							pobj->pParent->pChildren[c] = p;
							break;
						}
					}
				}
				else
					pRoot = p;

				for(c=0;c<Children;c++)
				{
					p->pChildren[c] = pobj->pChildren[c];
					pobj->pChildren[c] = NULL;
					if(p->pChildren[c]!=NULL)
					{
						p->pChildren[c]->pParent = p;
					}
				}

				SmallMove(&o->Obj);

				p->pParent = pobj->pParent;
				pobj->pParent = NULL;
				SmallMove(&p->Obj);
				//_AdjustSwap(p);
				break;
			}
		}
	}

	void _Unindex(Node *pobj)
	{
		unsigned int c = 0;
		for(unsigned int i = 0;i<Children;i++)
		{
			if(pobj->pChildren[i] != NULL)
				c++;
		}

		if(c==0)
			_UnindexNoChildren(pobj);
		else if(c==1)
			_Unindex1Child(pobj);
		else
			_UnindexChildren(pobj);

		Objects--;
	}

	RTree(unsigned int nodesize, int reserve) : heap(nodesize, reserve)
	{
		pRoot = NULL;
		Objects = 0;
		//MaxHeight = 0;
	}

	void _GetAllCollisions(const Key key, vector<A*> &ret, Node *p)
	{
		for(int c=0;c<Children;c++)
		{
			if(p->pChildren[c] != NULL)
			{
				if(p->pChildren[c]->Obj.IntersectRegion(key) )
				{
					if(p->pChildren[c]->Obj.IntersectObj(key) )
						ret.push_back(&p->pChildren[c]->Obj);
					_GetAllCollisions(key, ret, p->pChildren[c]);
				}
			}
		}
	}

public:

	RTree(int reserve) : heap((unsigned int)sizeof(Node), reserve)
	{
		pRoot = NULL;
		Objects = 0;
		//MaxHeight = 0;
	}

	/*template <class B>
	A *Create()
	{
		typedef RTreeObj<B,Key,Children, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), () );
		new (pobj) NodeB();
		_Insert((Node*)pobj);
		return (B*)pobj;
	}*/

	template <class B>
	A *Create(B tobj)
	{
		typedef RTreeObj<B,Key,Children, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (tobj) );
		new (pobj) NodeB(tobj);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	template <class B>
	A *CreateKey(const Key key)
	{
		typedef RTreeObj<B,Key,Children, sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) > sizeof(Node));
		NodeB *pobj = (NodeB*)heap.AllocBytes((unsigned int)sizeof(Node));//RayNew(heap, (LLObj<A,Key>), (key) );
		new (pobj) NodeB(key);
		_Insert((Node*)pobj);
		return (B*)pobj;
	}

	void GetAllCollisions(const Key &key, vector<A*> &ret)
	{
		if(pRoot != NULL && pRoot->Obj.IntersectRegion(key) )
		{
			if(pRoot->Obj.IntersectObj(key))
			{
				ret.push_back(&pRoot->Obj);
			}
			for(int c=0;c<Children;c++)
			{
				if(pRoot->pChildren[c] != NULL)
				{
					if(pRoot->pChildren[c]->Obj.IntersectRegion(key) )
					{
						if(pRoot->pChildren[c]->Obj.IntersectObj(key) )
							ret.push_back(&pRoot->pChildren[c]->Obj);
						_GetAllCollisions(key, ret, pRoot->pChildren[c]);
					}
				}
			}
		}
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

	void SmallMove(A *obj)
	{
		for(Node *pobj = (Node*)obj; pobj!=NULL; pobj=pobj->pParent)
		{
			Key OldRegion = pobj->Obj.GetRegion();
			pobj->Obj.ShrinkRegion();
			for(unsigned int c=0;c<Children;c++)
			{
				if(pobj->pChildren[c] != NULL)
					pobj->Obj.ResizeToRegion(&pobj->pChildren[c]->Obj);
			}

			if(OldRegion == pobj->Obj.GetRegion())
				break;
		}
	}

};


class Basic_2D_RTree_Object
{
public:
	RECT rect;
	RECT region;
	unsigned int area;

	Basic_2D_RTree_Object(RECT obj)
	{
		rect = region = obj;
		area = (rect.right-rect.left)*(rect.bottom-rect.top)/4;
	}

	__forceinline unsigned int GetChild(Basic_2D_RTree_Object *obj)
	{
		int myX = (region.left + region.right)/2;
		int myY = (region.top + region.bottom)/2;
		int oX = (obj->rect.left + obj->rect.right)/2;
		int oY = (obj->rect.top + obj->rect.bottom)/2;

		return (oX>myX) | ((oY>myY)<<1);
	}

	__forceinline unsigned int CostResizeToRect(RECT &obj)
	{
		int ret = 0;

		ret += (obj.left<region.left) * (region.left - obj.left);
		ret += (obj.right>region.right) * (obj.right - region.right);
		ret += (obj.top<region.top) * (region.top - obj.top);
		ret += (obj.bottom>region.bottom) * (obj.bottom - region.bottom);

		ret *= (region.right-region.left)+(region.bottom-region.top);

		return (unsigned int)ret;
	}

	__forceinline unsigned int CostResizeTo(Basic_2D_RTree_Object *pobj)
	{
		return CostResizeToRect(pobj->rect);
	}

	__forceinline unsigned int CostResizeToRegion(Basic_2D_RTree_Object *pobj)
	{
		return CostResizeToRect(pobj->region);
	}

	void ResizeTo(Basic_2D_RTree_Object *pobj)
	{
		region.left -= (pobj->rect.left<region.left) * (region.left - pobj->rect.left);
		region.right += (pobj->rect.right>region.right) * (pobj->rect.right - region.right);
		region.top -= (pobj->rect.top<region.top) * (region.top - pobj->rect.top);
		region.bottom += (pobj->rect.bottom>region.bottom) * (pobj->rect.bottom - region.bottom);
	}

	void ResizeToRegion(Basic_2D_RTree_Object *pobj)
	{
		region.left -= (pobj->region.left<region.left) * (region.left - pobj->region.left);
		region.right += (pobj->region.right>region.right) * (pobj->region.right - region.right);
		region.top -= (pobj->region.top<region.top) * (region.top - pobj->region.top);
		region.bottom += (pobj->region.bottom>region.bottom) * (pobj->region.bottom - region.bottom);
	}

	void ShrinkRegion()
	{
		region = rect;
	}

	void FillRegion()
	{
		rect = region;
	}

	RECT GetRegion()
	{
		return region;
	}

	bool IntersectObj(RECT key)
	{
		return ! ( key.left > rect.right
        || key.right < rect.left
        || key.top > rect.bottom
        || key.bottom < rect.top
        );
	}

	bool IntersectRegion(RECT key)
	{
		return ! ( key.left > region.right
        || key.right < region.left
        || key.top > region.bottom
        || key.bottom < region.top
        );
	}
};

/*is also defined in windows.h
bool operator==(RECT a, RECT b)
{
	return (a.left == b.left && a.right == b.right && a.top == b.top && a.bottom == b.bottom );
}
*/
