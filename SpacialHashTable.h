#pragma once
template<class A>
class ColPair
{
public:
	A *a;
	A *b;

	ColPair()
	{
	}

	ColPair(A* p1, A* p2)
	{
		a=p1;
		b=p2;
	}
};

template <class A, class Key, unsigned int padding=0>
class SpacialHashObj
{
public:
	A		Obj;
	char	Padding[padding];
	//int		X;
	//int		Y;

	SpacialHashObj()
	{
	}

	SpacialHashObj(A obj, int x, int y) : Obj(obj)
	{
		//X = x;
		//Y = y;
	}

	SpacialHashObj(Key key, int x, int y) : Obj(key)
	{
		//X = x;
		//Y = y;
	}

	~SpacialHashObj()
	{
	}
};

template <class A, class Key>
class SpacialHashObj<A,Key,0>
{
public:
	A	Obj;
	//int	X;
	//int	Y;

	SpacialHashObj()
	{
	}

	SpacialHashObj(A obj, int x, int y) : Obj(obj)
	{
		//X = x;
		//Y = y;
	}

	SpacialHashObj(Key key, int x, int y) : Obj(key)
	{
		//X = x;
		//Y = y;
	}

	~SpacialHashObj()
	{
	}
};

template <class A, class Key, unsigned int SLOTS, unsigned int SLOTSIZE, class LargeObj=A>
class SpacialHashTable : public Cont2
{
public:
	typedef SpacialHashObj<A,Key,sizeof(LargeObj)-sizeof(A)> Node;

	vector<Node> Slots[SLOTS][SLOTS];
	vector<Node> Borders;

	SpacialHashTable()
	{
	}

	template <class B>
	void Create(B tobj)
	{
		typedef SpacialHashObj<B,Key,sizeof(LargeObj)-sizeof(B)> NodeB;
		STATIC_ASSERT( sizeof(NodeB) != sizeof(Node));

		int x1,x2;
		int y1,y2;

		x1 = (int)tobj.rect.left;
		y1 = (int)tobj.rect.top;
		x2 = (int)tobj.rect.right;
		y2 = (int)tobj.rect.bottom;

		x1/=int(SLOTSIZE);
		y1/=int(SLOTSIZE);
		x2/=int(SLOTSIZE);
		y2/=int(SLOTSIZE);

		if( x2-x1<SLOTS && y2-y1<SLOTS )
		{
			((vector<NodeB>*)&Slots[(x1%SLOTS)][(y1%SLOTS)])->push_back( NodeB(tobj, x1, y1) );
		}
		/*else if( x1-x2<=1 && y1-y2<=1 )
		{
			((vector<NodeB>*)&Slots[(x1%SLOTS)][(y1%SLOTS)])->push_back( NodeB(tobj, x1, y1) );
		}*/
		else
		{
			((vector<NodeB>*)&Borders)->push_back( NodeB(tobj, x1, y1) );
		}
	}

	void GetAllCollisions(const Key &key, vector<A*> &ret)
	{
		int x1,x2;
		int y1,y2;

		x1 = (int)key.left;
		y1 = (int)key.top;
		x2 = (int)key.right;
		y2 = (int)key.bottom;

		x1/=int(SLOTSIZE);
		y1/=int(SLOTSIZE);
		x2/=int(SLOTSIZE);
		y2/=int(SLOTSIZE);

		if(x1==x2 && y1==y2)
		{
			for(int x=x1-1;x<=x1+1;x++)
			{
				for(int y=y1-1;y<=y1+1;y++)
				{
					vector<Node> *tvec = &Slots[x%SLOTS][y%SLOTS];

					for(unsigned int i=0;i<tvec->size();i++)
					{
						if( (*tvec)[i].Obj.IntersectObj(key) )
						{
							ret.push_back( &(*tvec)[i].Obj );
						}
					}
				}
			}

			vector<Node> *tvec = &Borders;
			for(unsigned int i=0;i<tvec->size();i++)
			{
				if( (*tvec)[i].Obj.IntersectObj(key) )
				{
					ret.push_back( &(*tvec)[i].Obj );
				}
			}
		}
		else
		{
			vector<Node> *tvec = &Borders;
			for(unsigned int i=0;i<tvec->size();i++)
			{
				if( (*tvec)[i].Obj.IntersectObj(key) )
				{
					ret.push_back( &(*tvec)[i].Obj );
				}
			}

			for(int x=x1-1;x<=x2+1;x++)
			{
				for(int y=y1-1;y<=y2+1;y++)
				{
					vector<Node> *tvec = &Slots[x1%SLOTS][y2%SLOTS];
					for(unsigned int i=0;i<tvec->size();i++)
					{
						if( (*tvec)[i].Obj.IntersectObj(key) )
						{
							ret.push_back( &(*tvec)[i].Obj );
						}
					}
				}
			}
		}
	}

	void Clear()
	{
		Borders.clear();
		for(int x=0;x<SLOTS;x++)
			for(int y=0;y<SLOTS;y++)
				Slots[x][y].clear();
	}

	void GetAllCollisionPairs(vector< ColPair<A> > &ret)
	{
		for(unsigned int i=0;i<Borders.size();i++)
		{
			for(int x=0;x<SLOTS;x++)
			{
				for(int y=0;y<SLOTS;y++)
				{
					vector<Node> *tvec = &Slots[x][y];
					for(unsigned int a=0;a<tvec->size();a++)
					{
						if( Borders[i].Obj.IntersectObj( (*tvec)[a].Obj.rect ) )
						{
							ret.push_back( ColPair<A>(&Borders[i].Obj, &(*tvec)[a].Obj) ); 
						}
					}
				}
			}

			for(unsigned int a=i+1;a<Borders.size();a++)
			{
				if( Borders[i].Obj.IntersectObj( Borders[a].Obj.rect ) )
				{
					ret.push_back( ColPair<A>(&Borders[i].Obj, &Borders[a].Obj) ); 
				}
			}
		}

		for(unsigned int x=0;x<SLOTS;x++)
		{
			for(unsigned int y=0;y<SLOTS;y++)
			{
				for(unsigned int i=0;i<Slots[x][y].size();i++)
				{
					for(unsigned int a=i+1;a<Slots[x][y].size();a++)
					{
						if( Slots[x][y][i].Obj.IntersectObj( Slots[x][y][a].Obj.rect ) )
						{
							ret.push_back( ColPair<A>(&Slots[x][y][i].Obj, &Slots[x][y][a].Obj) ); 
						}
					}

					int x1,x2;
					int y1,y2;

					x1 = (int)Slots[x][y][i].Obj.rect.left;
					y1 = (int)Slots[x][y][i].Obj.rect.top;
					x2 = (int)Slots[x][y][i].Obj.rect.right;
					y2 = (int)Slots[x][y][i].Obj.rect.bottom;

					x1/=int(SLOTSIZE);
					y1/=int(SLOTSIZE);
					x2/=int(SLOTSIZE);
					y2/=int(SLOTSIZE);

					for(int X=x1;X<=x2;X++)
					{
						for(int Y=y1-1;Y<=y2;Y++)
						{
							if(X!=x1 || Y>y1)
							{
								for(unsigned int a=0;a<Slots[X%SLOTS][Y%SLOTS].size();a++)
								{
									if( Slots[x][y][i].Obj.IntersectObj( Slots[X%SLOTS][Y%SLOTS][a].Obj.rect ) )
									{
										ret.push_back( ColPair<A>(&Slots[x][y][i].Obj, &Slots[X%SLOTS][Y%SLOTS][a].Obj) ); 
									}
								}
							}
						}
					}
				}
			}
		}
	}

};

class Basic_Spacial_Object
{
public:
	RECT rect;

	Basic_Spacial_Object(RECT obj)
	{
		rect = obj;
	}

	bool IntersectObj(RECT key)
	{
		return ! ( key.left > rect.right
        || key.right < rect.left
        || key.top > rect.bottom
        || key.bottom < rect.top
        );
	}

};

class fBasic_Spacial_Object
{
public:
	fRECT rect;

	fBasic_Spacial_Object(fRECT obj)
	{
		rect = obj;
	}

	fBasic_Spacial_Object()
	{
	}

	bool IntersectObj(fRECT key)
	{
		return ! ( key.left > rect.right
        || key.right < rect.left
        || key.top > rect.bottom
        || key.bottom < rect.top
        );
	}

};
