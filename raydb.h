class DBObj;
//defragmenting data
//	-at the start of each data section write the Index Number(not the Key, to account for multiple parts) and the length of the data
//	-to defrag, find the first gap in the data and move the next data segment back into it
//defragmenting index
//	-keep a linked list of IndexItems with a size of 0 or a NextPart < 0
//	-every idle tick move the bottom child(default to right side?) of a deleted index item to be a child of the deleted item's parent
//		-must keep the original parent of the deleted item from before defraging
//	-when the deleted item is at the bottom of the tree(it has no children), remove it from the tree and add it to a linked list of empty spaces in the index file
//	-when creating an item allow it's index to fill one of the slots in the removed index items list
//	-make sure that when switching an index item with index item 0 that you physically switch them in the file, because the root node always needs to be item 0
//		-also make sure to change what the indecies say on the data segments

#pragma pack(4)
struct IndexItem
{
	unsigned __int64 Key[2];
	unsigned __int64 Index;
	//unsigned int iSize;//4 GB Max Data Segment, maybe I can add support to split up one item into multiple pieces if it's over 4 GB, shouldn't be needed though
	int Left;
	int Right;
	//int NextPart;
};

#pragma pack(4)
struct IndexHeader
{//max of 2,147,483,647 rows
	__int64 CurrUpdate;
	int NumRows;//this includes data fragments
	int NumItems;//this does not
};

#pragma pack(4)
struct DataHeader
{
	unsigned __int64 Key[2];
	int iSize;
};

class RayDB
{
public:
	HANDLE index;
	HANDLE dbfile;
	char DBIName[128];
	char DBName[128];
	unsigned __int64 Keys[2];
	__int64	temppos;
	int tempsize;
	int LastParent;
	int szRow;
	int IndexBuffered;
	int NumRowsBuffered;
	__int64 UpdateBuffered;//count DB updates
	int KeepIndexBuffered;
	int Lefts;
	int Rights;
	int NeedsDefrag;
	unsigned long	Written;
	LARGE_INTEGER LargeInt;
	LARGE_INTEGER LastDefrag;

	IndexHeader IH;

	IndexItem* IndexBuffer;
	IndexItem TempIndItem;
	DataHeader TempDataHeader;
	int MAXDBIBUFFER;

	__inline int GetNextIndexSlot()
	{
		IH.NumRows++;
		return IH.NumRows-1;
	}

	void SaveIH()
	{
		LargeInt.QuadPart = 0;
		SetFilePointerEx(index,LargeInt,0,FILE_BEGIN);
		WriteFile(index,&IH,sizeof(IH),&Written,0);
		//FlushFileBuffers(index);

		if(KeepIndexBuffered==2)
			BufferIndex();
	}

	void IncrementCurrUpdate()
	{
		IH.CurrUpdate++;
		LargeInt.QuadPart = 0;
		SetFilePointerEx(index,LargeInt,0,0);
		WriteFile(index,&IH,sizeof(IH),&Written,0);

		if(KeepIndexBuffered==2)
			BufferIndex();
	}

	void SaveIndexItem(int a)
	{
		if(a < NumRowsBuffered)
		{
			IndexBuffer[a] = TempIndItem;
		}
		//save to file...

		LargeInt.QuadPart = (a*sizeof(IndexItem)) + sizeof(IH);
		SetFilePointerEx(index,LargeInt,0,0);
		WriteFile(index,&TempIndItem,sizeof(IndexItem),&Written,0);

		//FlushFileBuffers(index);
	}

	void LoadIndexItem(int a)
	{
		if(a < NumRowsBuffered && UpdateBuffered == IH.CurrUpdate)
		{
			TempIndItem = IndexBuffer[a];
			return;
		}
		else if(a < MAXDBIBUFFER && IndexBuffered == 1 && IH.CurrUpdate == UpdateBuffered && KeepIndexBuffered>0)
		{
			LargeInt.QuadPart = (NumRowsBuffered*sizeof(IndexItem))+sizeof(IH);
			SetFilePointerEx(index,LargeInt,0,0);
			ReadFile(index,&IndexBuffer[NumRowsBuffered],(a+1-NumRowsBuffered)*sizeof(IndexItem),&Written,0);
			IndexBuffered = 1;
			NumRowsBuffered = a+1;

			TempIndItem = IndexBuffer[a];
			return;
		}
		else if(a < MAXDBIBUFFER && KeepIndexBuffered>0)
		{
			if(IndexBuffer != 0)
				delete[] IndexBuffer;
			IndexBuffer = new IndexItem[MAXDBIBUFFER];
			NumRowsBuffered = a+1;

			LargeInt.QuadPart = sizeof(IH);
			SetFilePointerEx(index,LargeInt,0,0);
			ReadFile(index, IndexBuffer, NumRowsBuffered*sizeof(IndexItem),&Written, 0);
			IndexBuffered = 1;
			UpdateBuffered = IH.CurrUpdate;

			TempIndItem = IndexBuffer[a];
			return;
		}
		else
		{
			LargeInt.QuadPart = (a*sizeof(IndexItem)) + sizeof(IH);
			SetFilePointerEx(index,LargeInt,0,0);
			ReadFile(index,&TempIndItem,sizeof(IndexItem),&Written,0);
		}
	}

	int CrawlIndexItem2(int from, int child)
	{
		int child2;
		LoadIndexItem(from);
		LastParent = from;
		if(child==0)
		{
			if(TempIndItem.Left == -1)
			{
				temppos = -1;
				return -1;
			}
			child2 = TempIndItem.Left;
		}
		else
		{
			if(TempIndItem.Right == -1)
			{
				temppos = -1;
				return -1;
			}
			child2 = TempIndItem.Right;
		}

		if(child2 >= IH.NumRows)
		{
			temppos = -1;
			return -1;
		}
		LoadIndexItem(child2);

		temppos = TempIndItem.Index;
		return child2;
		//return -1;
	}

	int CrawlIndexItem(int from, int child)
	{
		int child2;
		LoadIndexItem(from);
		LastParent = from;
		if(child==0)
		{
			if(TempIndItem.Left == -1)
			{
				temppos = -1;
				return -1;
			}
			child2 = TempIndItem.Left;
		}
		else
		{
			if(TempIndItem.Right == -1)
			{
				temppos = -1;
				return -1;
			}
			child2 = TempIndItem.Right;
		}

		if(child2 >= IH.NumRows)
		{
			temppos = -1;
			return -1;
		}
		LoadIndexItem(child2);

		{
			temppos = TempIndItem.Index;
			return child2;
		}
		return -1;
	}

	int FindIndexItem()
	{
		int a=0;
		temppos = 0;
		LastParent=-1;

		LoadIndexItem(a);
		while(1)
		{
			if(a >= IH.NumRows || a == -1  || (TempIndItem.Key[0] == 0 && TempIndItem.Key[1] == 0))
			{
				temppos = -1;
				return -1;
			}

			if(TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] == Keys[1])
			{
				return a;
			}
			else if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
			{//go left
				a = CrawlIndexItem(a, 0);
			}
			else
			{//go right
				a = CrawlIndexItem(a, 1);
			}
		}
	}

	int FindFreeParent2()
	{
		int a=0;
		int e =0;
		temppos = 0;
		LastParent=-1;

		while(1)
		{
			if(a >= IH.NumRows || a==-1)
			{
				return a;
			}

			LoadIndexItem(a);
			if((TempIndItem.Key[0] == 0 && TempIndItem.Key[1] == 0))
				return -1;

			else if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
			{//go left
				e = CrawlIndexItem2(a, 0);

				if(e==-1)
				{
					return a;
				}
				else
					a = e;
			}
			else
			{//go right
				e = CrawlIndexItem2(a, 1);

				if(e==-1)
				{
					return a;
				}
				else
					a = e;
			}
		}
	}


	int FindFreeParent()
	{
		int a=0;
		int e =0;
		temppos = 0;
		LastParent=-1;

		while(1)
		{
			if(a >= IH.NumRows || a==-1)
			{
				return a;
			}

			LoadIndexItem(a);
			if((TempIndItem.Key[0] == 0 && TempIndItem.Key[1] == 0))
				return -1;

			if(TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] == Keys[1])
			{
				return -1;
			}
			else if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
			{//go left
				e = CrawlIndexItem2(a, 0);

				if(e==-1)
				{
					return a;
				}
				else
					a = e;
			}
			else
			{//go right
				e = CrawlIndexItem2(a, 1);

				if(e==-1)
				{
					return a;
				}
				else
					a = e;
			}
		}
	}


	void DeleteItem(unsigned __int64 Key0, unsigned __int64 Key1)
	{
		int item;
		int item2;

		Keys[0] = Key0;
		Keys[1] = Key1;
		item = FindIndexItem();

		if(item == -1)
			return;

		if(TempIndItem.Left == -1 && TempIndItem.Right == -1)
		{
			if(LastParent != -1)
			{
				LoadIndexItem(LastParent);
				if(TempIndItem.Left == item)
					TempIndItem.Left = -1;
				else if(TempIndItem.Right == item)
					TempIndItem.Right = -1;
				SaveIndexItem(LastParent);
			}
			if(item != IH.NumRows-1)
			{
				item2 = IH.NumRows-1;
				LoadIndexItem(item2);
				Keys[0] = TempIndItem.Key[0];
				Keys[1] = TempIndItem.Key[1];
				item2 = FindIndexItem();
				SaveIndexItem(item);
				if(LastParent!=-1)
				{
					LoadIndexItem(LastParent);

					if(TempIndItem.Left == item2)
						TempIndItem.Left = item;
					else if(TempIndItem.Right == item2)
						TempIndItem.Right = item;
					SaveIndexItem(LastParent);
				}
			}
		}
		else if(TempIndItem.Left == -1 || TempIndItem.Right == -1)
		{
			if(TempIndItem.Left == -1)
				item2 = TempIndItem.Right;
			else
				item2 = TempIndItem.Left;
			LoadIndexItem(item2);
			SaveIndexItem(item);

			if(item2 != IH.NumRows-1)
			{
				item = IH.NumRows-1;
				LoadIndexItem(item);
				Keys[0] = TempIndItem.Key[0];
				Keys[1] = TempIndItem.Key[1];
				item = FindIndexItem();
				SaveIndexItem(item2);
				if(LastParent!=-1)
				{
					LoadIndexItem(LastParent);

					if(TempIndItem.Left == item)
						TempIndItem.Left = item2;
					else if(TempIndItem.Right == item)
						TempIndItem.Right = item2;
					SaveIndexItem(LastParent);
				}
			}
		}
		else
		{
			IndexItem TIndItem;// = TempIndItem;
			//IndexItem TIndItem2;
			if(rand()&1)
			{
				item2 = TempIndItem.Left;
				LoadIndexItem(TempIndItem.Left);
				while(TempIndItem.Right!=-1)
				{
					item2 = TempIndItem.Right;
					LoadIndexItem(TempIndItem.Right);
				}
			}
			else
			{
				item2 = TempIndItem.Right;
				LoadIndexItem(TempIndItem.Right);
				while(TempIndItem.Left!=-1)
				{
					item2 = TempIndItem.Left;
					LoadIndexItem(TempIndItem.Left);
				}
			}

			TIndItem = TempIndItem;

			DeleteItem(TIndItem.Key[0], TIndItem.Key[1]);

			Keys[0] = Key0;
			Keys[1] = Key1;
			item = FindIndexItem();

			TIndItem.Left = TempIndItem.Left;
			TIndItem.Right = TempIndItem.Right;

			TempIndItem = TIndItem;

			SaveIndexItem(item);

			return;
		}

		IH.NumRows--;
		IH.NumItems--;

		SaveIH();
		LargeInt.QuadPart = 0;
		SetFilePointerEx(index,LargeInt,&LargeInt,FILE_END);
		LargeInt.QuadPart -= sizeof(IndexItem);
		SetFilePointerEx(index,LargeInt,0,0);
		SetEndOfFile(index);
	}

	void AddItem(char* Data, unsigned __int64 Key0, unsigned __int64 Key1, int size)
	{
		Keys[0] = Key0;
		Keys[1] = Key1;

		int slot;
		if(IH.NumRows>0)
		{
			int slot2 = FindFreeParent();
			//cout << "FindFreeParent() == "<<slot<<"\n";
			if(slot2==-1)
			{
				cout <<"Error! Item "<<Key0<<", "<<Key1<<" already exists!\n";
				//Sleep(100);
				return;
			}
			slot=GetNextIndexSlot();
			LoadIndexItem(slot2);
			if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
			{
				TempIndItem.Left = slot;
				Lefts++;
				//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
			}
			else
			{
				TempIndItem.Right = slot;
				Rights++;
				//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
			}
			SaveIndexItem(slot2);
		}
		else
			slot = GetNextIndexSlot();

		LargeInt.QuadPart = 0;
		SetFilePointerEx(dbfile,LargeInt,&LargeInt,FILE_END);
		TempIndItem.Index = LargeInt.QuadPart;
		//TempIndItem.iSize = size;
		TempIndItem.Key[0] = Key0;
		TempIndItem.Key[1] = Key1;
		TempIndItem.Left = -1;
		TempIndItem.Right = -1;
		//TempIndItem.NextPart = 0;

		SaveIndexItem(slot);

		IH.NumItems++;

		TempDataHeader.iSize = size;
		TempDataHeader.Key[0] = Key0;
		TempDataHeader.Key[1] = Key1;
		WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
		WriteFile(dbfile, Data, size, &Written, 0);

		SaveIH();
	}

	void UpdateItem(char* Data, unsigned __int64 Key0, unsigned __int64 Key1, int size)//oh boy...., maybe just delete and re-add, defrag can clean up the mess?
	{
		Keys[0] = Key0;
		Keys[1] = Key1;
		int item = FindIndexItem();
		if(item==-1)
			return;

		LargeInt.QuadPart = TempIndItem.Index;
		SetFilePointerEx(dbfile, LargeInt,0,0);
		ReadFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);

		if(size == 0)
		{
			//mark old area as fragmented
			TempDataHeader.iSize *= -1;
			LargeInt.QuadPart = TempIndItem.Index;
			SetFilePointerEx(dbfile, LargeInt,0,0);
			WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);

			//delete index item from binary tree
			DeleteItem(Key0, Key1);
		}
		else if(size == TempDataHeader.iSize)
		{
			LargeInt.QuadPart = TempIndItem.Index;
			SetFilePointerEx(dbfile, LargeInt,0,0);
			WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
			WriteFile(dbfile,Data,size, &Written,0);
		}
		else
		{
			//mark old area as fragmented
			TempDataHeader.iSize *= -1;
			LargeInt.QuadPart = TempIndItem.Index;
			SetFilePointerEx(dbfile, LargeInt,0,0);
			WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);

			TempDataHeader.iSize = size;
			SetFilePointerEx(dbfile,LargeInt,&LargeInt,FILE_END);
			TempIndItem.Index = LargeInt.QuadPart;
			SaveIndexItem(item);
			WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
			WriteFile(dbfile,Data,size, &Written,0);
		}
	}

	int Maintenance()
	{//run this once every time the DB checks the query queue and it is empty
		//buffer...
		BufferIndex();//should I buffer before, after, or both? probably before, updates are applied to the buffered copies anyways
		//LastDefrag
		LARGE_INTEGER DefragTo;

		GetFileSizeEx(dbfile, &DefragTo);
		if(LastDefrag.QuadPart >= DefragTo.QuadPart)
		{
			LastDefrag.QuadPart = 0;
			return 0;
		}

		SetFilePointerEx(dbfile, LastDefrag, &LastDefrag, 0);
		DefragTo = LastDefrag;
		ReadFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
		if(TempDataHeader.iSize>=0)
		{
			LastDefrag.QuadPart += sizeof(DataHeader) + TempDataHeader.iSize;
			return 0;
		}
		while(TempDataHeader.iSize<0)
		{
			LastDefrag.QuadPart = TempDataHeader.iSize * -1;
			SetFilePointerEx(dbfile, LastDefrag, &LastDefrag, FILE_CURRENT);
			ReadFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
			if(Written < sizeof(DataHeader))
			{
				SetFilePointerEx(dbfile, DefragTo, &LastDefrag, 0);
				SetEndOfFile(dbfile);
				LastDefrag.QuadPart = 0;
				return 0;
			}
		}
		char *tmpchr = new char[TempDataHeader.iSize];
		ReadFile(dbfile, tmpchr, TempDataHeader.iSize, &Written, 0);
		SetFilePointerEx(dbfile, DefragTo, 0, 0);

		Keys[0] = TempDataHeader.Key[0];
		Keys[1] = TempDataHeader.Key[1];
		int item = FindIndexItem();
		TempIndItem.Index = DefragTo.QuadPart;
		SaveIndexItem(item);

		WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
		WriteFile(dbfile, tmpchr, TempDataHeader.iSize, &Written, 0);

		TempDataHeader.iSize = (LastDefrag.QuadPart - DefragTo.QuadPart)*-1 + sizeof(DataHeader);

		WriteFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);
		LastDefrag.QuadPart=0;
		SetFilePointerEx(dbfile, LastDefrag, &LastDefrag, FILE_CURRENT);
		LastDefrag.QuadPart -= sizeof(DataHeader);
		return 1;
	}

	void BufferIndex()
	{
		int a;
		a = MAXDBIBUFFER-1;
		if(a >= IH.NumRows)
			a=IH.NumRows;
		LoadIndexItem(a);
		return;
	}

	char* GetItem(unsigned __int64 Key0, unsigned __int64 Key1)
	{
		Keys[0] = Key0;
		Keys[1] = Key1;
		int a;
		a = FindIndexItem();

		if(temppos == -1)
			return 0;

		LargeInt.QuadPart = TempIndItem.Index;
		SetFilePointerEx(dbfile, LargeInt,0,0);
		ReadFile(dbfile, &TempDataHeader, sizeof(DataHeader), &Written, 0);

		this->tempsize = TempDataHeader.iSize;
		char* TempObj = new char[TempDataHeader.iSize];

		ReadFile(dbfile, TempObj, TempDataHeader.iSize, &Written, 0);

		return TempObj;//make sure to delete[] this when you're done!
	}

	RayDB(char* dbname)
	{
		Lefts = Rights = 0;
		_mkdir("DBI");
		_mkdir("DB");
		sprintf_s(DBIName,128,"DBI\\%s.rdbi",dbname);
		sprintf_s(DBName,128,"DB\\%s.rdb",dbname);

		index = CreateFileA(DBIName, GENERIC_ALL, FILE_SHARE_READ, 0, CREATE_NEW, FILE_FLAG_RANDOM_ACCESS, 0);

		int i = GetLastError();
		if(i == 80)
		{
			CloseHandle(index);
			index = CreateFileA(DBIName, GENERIC_ALL, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
		}

		dbfile = CreateFileA(DBName, GENERIC_ALL, FILE_SHARE_READ, 0, CREATE_NEW, FILE_FLAG_RANDOM_ACCESS, 0);

		i = GetLastError();
		if(i == 80)
		{
			CloseHandle(dbfile);
			dbfile = CreateFileA(DBName, GENERIC_ALL, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
		}

		if(index == INVALID_HANDLE_VALUE)
			cout << "Bad Index File!\n";
		if(dbfile == INVALID_HANDLE_VALUE)
			cout << "Bad DB File!\n";

		NeedsDefrag = 2;
		LastDefrag.QuadPart = 0;
		szRow=0;
		IndexBuffer = 0;
		IndexBuffered = 0;
		NumRowsBuffered = 0;
		KeepIndexBuffered = 1;
		MAXDBIBUFFER = ((1024*1024) * 10) / sizeof(IndexItem);//10MB of RAM
		MAXDBIBUFFER = 4;

		GetFileSizeEx(index, &LargeInt);
		if(LargeInt.QuadPart < sizeof(IH))
		{
			IH.CurrUpdate = 1;
			IH.NumRows = 0;
			IH.NumItems = 0;
			this->SaveIH();
			return;
		}
		LargeInt.QuadPart = 0;
		SetFilePointerEx(index,LargeInt,0,0);
		if(ReadFile(index, &IH, sizeof(IH), &Written, 0)==1)
		{
			this->SaveIH();
		}
		else
		{
			i = GetLastError();
		}
		BufferIndex();
	}
};

class DBObj
{
public:
	unsigned __int64 Key[2];

	virtual void GenKey()=0;
	virtual int GetSize()
	{
		return sizeof(*this);//note: methods like this depend on the size of the virtual function table pointer, 32bit DBs will not be compatible with a 64bit build and vice versa
	}

	virtual void Load(char* Data)
	{
		if(Data==NULL)
		{
			memset(this,0,this->GetSize());//note: methods like this depend on the size of the virtual function table pointer, 32bit DBs will not be compatible with a 64bit build and vice versa
			return;
		}
		memcpy(this, Data,this->GetSize());
		delete[] Data;
	}

	virtual char* Save()
	{
		return (char*)this;//note: methods like this depend on the size of the virtual function table pointer, 32bit DBs will not be compatible with a 64bit build and vice versa
	}
};