
template<class A>
class ContObj2 : public ContObj<A>
{
public:
	//int Objects;
	int LObjects;
	int RObjects;

	ContObj2()
	{
		LObjects=0;
		RObjects=0;
	}

	void CalcObjects()
	{
		LObjects=0;
		RObjects=0;
		if(pLeft!=NULL)
		{
			if(pLeft->LObjects > pLeft->RObjects)
				LObjects=pLeft->LObjects+1;
			else
				LObjects=pLeft->RObjects+1;
		}
		if(pRight!=NULL)
		{
			if(pRight->LObjects > pRight->RObjects)
				RObjects=pRight->LObjects+1;
			else
				RObjects=pRight->RObjects+1;
		}
	}
};

template<class A>
class AVLTree : public Hash<A>
{
public:
	//set the object count after inserts, deletes, and rotations
	//(it's easy, for every object affected do Objects = pLeft->Objects + pRight->Objects + the direct children)
	
	virtual void RotLeft(A *Obj)
	{
		Hash::RotLeft(Obj);
		Obj->CalcObjects();
		if(Obj->pParent!=NULL)
			Obj->pParent->CalcObjects();
	}

	virtual void RotRight(A *Obj)
	{
		Hash::RotRight(Obj);
		Obj->CalcObjects();
		if(Obj->pParent!=NULL)
			Obj->pParent->CalcObjects();
	}

	virtual void Balance(A *pTemp)
	{
		for(;pTemp!=NULL;pTemp=pTemp->pParent)
		{
			pTemp->CalcObjects();
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
	}

	virtual void Create(A* currobj)
	{
		Hash::Create(currobj);
		currobj->CalcObjects();
		//A *pTemp=currobj->pParent;
		Balance(currobj->pParent);
	}

	virtual void Del(A* currobj)
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
				Balance(currobj->pParent);
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

					currobj->pRight->pParent = currobj->pParent;
					Balance(currobj->pParent);
				}
				else
					currobj->pRight->pParent = NULL;
					
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
					currobj->pLeft->pParent = currobj->pParent;
					Balance(currobj->pParent);
				}
				else
					currobj->pLeft->pParent = NULL;
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
};
