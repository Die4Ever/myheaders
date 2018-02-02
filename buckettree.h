template <class A, int Bs>
class BucketObj
{
public:
	char	*name;
	A		*pChildren[Bs];
	A		*pParent;
	RayCont<A> *Cont;

	int		namelen;

	virtual ~BucketObj()
	{
		if(name!=NULL)
			delete[] name;
		name = 0;
		Del();
	}

	BucketObj()
	{
		pParent = NULL;
		Cont = NULL;
		name = NULL;
		namelen = 0;
		memset( pChildren, 0, sizeof(void*)*Bs);
	}

	static int GetBs()
	{
		return Bs;
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

	int SortComp(A* objB)
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

	unsigned int GetBucket(int level)
	{
		if(name == NULL)
			return 0;
		if(namelen <= level)
			return 0;

		if( name[level] >= 'a' && name[level] <= 'z' )
			return (name[level] - 'a')/2 +6;
		if( name[level] >= 'A' && name[level] <= 'Z' )
			return (name[level] - 'A')/2 +6;
		if( name[level] >= '0' && name[level] <= '9' )
			return (name[level] - '0')/2 + 1;

		/*if( name[level] >= 'a' && name[level] <= 'z' )
			return name[level] - ('a'-11);
		if( name[level] >= 'A' && name[level] <= 'Z' )
			return name[level] - ('A'-11);
		if( name[level] >= '0' && name[level] <= '9' )
			return name[level] - ('0'-1);*/
		return 0;
	}

	static unsigned int GetBucket(char *name, int level, int namelen=-10)
	{
		if(name == NULL)
			return 0;
		if(namelen==-10)
			namelen = GetNameLen(name);
		if(namelen <= level)
			return 0;

		if( name[level] >= 'a' && name[level] <= 'z' )
			return (name[level] - 'a')/2 +6;
		if( name[level] >= 'A' && name[level] <= 'Z' )
			return (name[level] - 'A')/2 +6;
		if( name[level] >= '0' && name[level] <= '9' )
			return (name[level] - '0')/2 + 1;

		/*if( name[level] >= 'a' && name[level] <= 'z' )
			return name[level] - ('a'-11);
		if( name[level] >= 'A' && name[level] <= 'Z' )
			return name[level] - ('A'-11);
		if( name[level] >= '0' && name[level] <= '9' )
			return name[level] - ('0'-1);*/
		return 0;
	}

	/*unsigned int GetBucket(int level)
	{
		if(name==NULL)
		{
			return 0;
		}
		if(namelen <= level)
		{
			return 0;
			//unsigned int ret = ((unsigned char*)name)[(level%namelen)] % Bs;
			//return ret;
		}
		//return ( ( ((unsigned char*)name)[level>>2]>>(level & 1)) & 0xf0) >> 4;

		return ((name)[level] & 0xf8) >> 3;
		if( level & 1 == 0 )
			return (((unsigned char*)name)[level>>2] & 0xf0)>>4;
		else
			return (((unsigned char*)name)[level>>2] & 0x0f);

		//return ((unsigned char*)name)[level] % Bs;
		//return ret;
	}*/

	static int GetNameLen(char *name)
	{
		return strlen(name);
	}

	/*static unsigned int GetBucket(char *tname, int level, int namelen=-10)
	{
		if(tname==NULL)
		{
			return 0;
		}
		if(namelen == -10)
			namelen = GetNameLen(tname);
		if(namelen <= level)
		{
			return 0;
			//unsigned int ret = ((unsigned char*)tname)[(level%namelen)] % Bs;
			//return ret;
		}
		//return ( ( ((unsigned char*)tname)[level>>2]>>(level & 1)) & 0xf0) >> 4;

		return ((tname)[level] & 0xf8) >> 3;
		if( level & 1 == 0 )
			return (((unsigned char*)tname)[level>>2] & 0xf0)>>4;
		else
			return (((unsigned char*)tname)[level>>2] & 0x0f);
		//return ((unsigned char*)tname)[level] % Bs;
		//return ret;
	}*/
};

template<class A>
class BucketTree : public RayCont<A>
{
public:
	int iMaxLev;
	BucketTree()
	{
		iMaxLev = 0;
	}

	virtual void Create(A* currobj)
	{
		currobj->Cont = this;
		Objects++;
		if(pRoot == NULL)
		{
			pRoot = pTail = currobj;
			return;
		}

		A *pTemp;
		A *pTemp2;
		int iLevel = 0;
		unsigned int buck;
		for(pTemp = pRoot; pTemp != NULL; iLevel++)
		{
			buck = currobj->GetBucket(iLevel);
			/*if( pTemp->pChildren[buck] == NULL )
			{
				pTemp->pChildren[buck] = currobj;
				currobj->pParent = pTemp;
				if(iLevel > iMaxLev)
					iMaxLev = iLevel;
				return;
			}*/
			pTemp2 = pTemp;
			pTemp = pTemp->pChildren[buck];
			//stepcount++;
		}
		pTemp2->pChildren[buck] = currobj;
		currobj->pParent = pTemp2;
		if(iLevel > iMaxLev)
			iMaxLev = iLevel;
	}

	void Del(A* currobj)
	{
		int slot = rand()%currobj->GetBs();
		A *pTemp = currobj;
		int i=0;
		for(; ; pTemp = pTemp->pChildren[slot])
		{
			if(pTemp->pChildren[slot] == NULL)
			{
				for(i=0;i<pTemp->GetBs(); i++)
				{
					if( pTemp->pChildren[i] != NULL)
					{
						slot = i;
						break;
					}
				}
				if(i == pTemp->GetBs())
				{
					pTemp->pParent->pChildren[slot] = NULL;
					pTemp->pParent = currobj->pParent;
					for(int a=0;currobj->pParent != NULL && a<currobj->pParent->GetBs(); a++)
					{
						if( currobj->pParent->pChildren[a] == currobj )
						{
							currobj->pParent->pChildren[a] = pTemp;
							break;
						}
					}
					if( this->pRoot == currobj)
						this->pRoot = pTemp;
					if( this->pTail == currobj)
						this->pTail = pTemp;
					if( this->pTemp == currobj)
						this->pTemp = pTemp;
					for(int a=0; a<currobj->GetBs(); a++)
					{
						pTemp->pChildren[a] = currobj->pChildren[a];
						if(pTemp->pChildren[a] != NULL)
							pTemp->pChildren[a]->pParent = pTemp;
					}
				}
			}
		}
	}

	A* Get(int &i,A *Start)
	{
		return 0;
	}

	A* Get(char *tempname)
	{
		A *pTemp;
		int iLevel = 0;
		unsigned int buck;
		int namelen = A::GetNameLen(tempname);
		for(pTemp = pRoot; pTemp!=NULL; iLevel++)
		{
			//stepcount++;
			if( pTemp->SortComp(tempname) == 0)
			{
				return pTemp;
			}
			buck = A::GetBucket(tempname, iLevel, namelen);
			pTemp = pTemp->pChildren[buck];
		}
		return 0;
	}

	A* OrderedGet(int &i, A *Start)
	{
		return 0;
	}

	A* CopyObj(A* currobj)
	{
		return 0;
	}

	char* DisplayList()
	{
		return 0;
	}

	void Sort()
	{
	}

	void Clear(A *Start)
	{
	}
};