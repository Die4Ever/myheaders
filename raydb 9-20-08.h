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

LARGE_INTEGER LargeInt;

#pragma pack(4)
struct IndexItem
{
	unsigned __int64 Key[2];
	unsigned __int64 Index;
	unsigned int iSize;//4 GB Max Data Segment, maybe I can add support to split up one item into multiple pieces if it's over 4 GB, shouldn't be needed though
	int Left;
	int Right;
	int NextPart;
};

#pragma pack(4)
struct IndexHeader
{//max of 2,147,483,647 rows
	__int64 CurrUpdate;
	int NumRows;//this includes data fragments
	int NumItems;//this does not
};

#pragma pack(4)
struct DefragIndex
{
	int Index;
	int Parent;
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
	int szRow;
	int IndexBuffered;
	int NumRowsBuffered;
	int NumRowsBackBuffered;
	__int64 UpdateBuffered;//count DB updates
	int KeepIndexBuffered;
	int Lefts;
	int Rights;
	int DefragIndecies;
	int DefragData;
	int LastMaintItem;
	unsigned long	Written;

	IndexHeader IH;

	IndexItem* IndexBuffer;
	IndexItem* IndexBackBuffer;
	IndexItem TempIndItem;
	int MAXDBIBUFFER;

	int GetNextIndexSlot()
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
				tempsize = TempIndItem.iSize;
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
		//....gonna have to resort the tree, or have a special flag for a deleted item while still keeping the same key, I could set the size to negative....
		IncrementCurrUpdate();
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
		TempIndItem.iSize = size;
		TempIndItem.Key[0] = Key0;
		TempIndItem.Key[1] = Key1;
		TempIndItem.Left = -1;
		TempIndItem.Right = -1;
		TempIndItem.NextPart = 0;

		SaveIndexItem(slot);

		IH.NumItems++;

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

		if(size != TempIndItem.iSize)
		{
			int i = item;
			int i2 = item;
			int ind=0;
			if(TempIndItem.NextPart<0)
			{
				while(TempIndItem.NextPart<0)
				{
					i = TempIndItem.NextPart*-1;
					LoadIndexItem(i);
				}
			}
			while(1)
			{
				LargeInt.QuadPart = TempIndItem.Index;
				SetFilePointerEx(dbfile, LargeInt, 0, 0);
				if(size < TempIndItem.iSize + ind)
				{
					int slot;
					int slot2;
					while(TempIndItem.NextPart!=0)
					{
						if(TempIndItem.NextPart>0)
						{
							TempIndItem.NextPart *= -1;
							SaveIndexItem(i);
						}
						i = TempIndItem.NextPart*-1;
						LoadIndexItem(i);
					}
					slot2 = GetNextIndexSlot();
					TempIndItem.NextPart = slot2 * -1;
					if(i<0)
						i*=-1;
					SaveIndexItem(i);

					slot = FindFreeParent2();
					//cout << "FindFreeParent() == "<<slot<<"\n";
					if(slot==-1)
					{
						cout <<"Error! Item already exists!\n";
						return;
					}
					LoadIndexItem(slot);
					if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
					{
						TempIndItem.Left = slot2;
						Lefts++;
						//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
					}
					else
					{
						TempIndItem.Right = slot2;
						Rights++;
						//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
					}
					SaveIndexItem(slot);
					slot = slot2;
					SaveIH();

					LoadIndexItem(slot);
					TempIndItem.Key[0] = Keys[0];
					TempIndItem.Key[1] = Keys[1];
					TempIndItem.iSize = size - ind;
					TempIndItem.Left = -1;
					TempIndItem.Right = -1;
					TempIndItem.NextPart = 0;
					LargeInt.QuadPart = 0;
					SetFilePointerEx(dbfile, LargeInt, &LargeInt, FILE_END);
					TempIndItem.Index = LargeInt.QuadPart;
					SaveIndexItem(slot);

					if(size - ind > 0)
						WriteFile(dbfile, &Data[ind],size-ind,&Written,0);
					break;
				}
				else if(TempIndItem.iSize > 0)
					WriteFile(dbfile, &Data[ind],TempIndItem.iSize,&Written,0);
				ind+= TempIndItem.iSize;
				if(size - ind == 0)
					break;
				if(TempIndItem.NextPart<0)
				{
					while(TempIndItem.NextPart<0)
					{
						i = TempIndItem.NextPart*-1;
						LoadIndexItem(i);
					}
				}
				else if(TempIndItem.NextPart == 0)
				{
					int slot;
					int slot2;
					slot2 = GetNextIndexSlot();
					TempIndItem.NextPart = slot2;
					if(TempIndItem.iSize == 0)
						TempIndItem.NextPart *= -1;
					if(i<0)
						i*=-1;
					SaveIndexItem(i);

					slot = FindFreeParent2();
					if(slot==-1)
					{
						cout <<"Error! Item already exists!\n";
						return;
					}
					LoadIndexItem(slot);
					if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
					{
						TempIndItem.Left = slot2;
						Lefts++;
						//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
					}
					else
					{
						TempIndItem.Right = slot2;
						Rights++;
						//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
					}
					SaveIndexItem(slot);
					slot = slot2;
					SaveIH();

					LoadIndexItem(slot);
					TempIndItem.Key[0] = Keys[0];
					TempIndItem.Key[1] = Keys[1];
					TempIndItem.iSize = size - ind;
					TempIndItem.Left = -1;
					TempIndItem.Right = -1;
					TempIndItem.NextPart = 0;

					LargeInt.QuadPart = 0;
					SetFilePointerEx(dbfile, LargeInt, &LargeInt, FILE_END);

					TempIndItem.Index = LargeInt.QuadPart;
					SaveIndexItem(slot);

					WriteFile(dbfile, &Data[ind],size-ind,&Written,0);
					break;
				}
				else
				{
					i = TempIndItem.NextPart;
					LoadIndexItem(TempIndItem.NextPart);
				}
			}
		}
		else if(size == TempIndItem.iSize)
		{//lol, thank god
			LargeInt.QuadPart = TempIndItem.Index;
			SetFilePointerEx(dbfile, LargeInt,0,0);
			WriteFile(dbfile,Data,size, &Written,0);
		}
	}

	void Maintenance()
	{//run this once every time the DB checks the query queue and it is empty
		//buffer...
		BufferIndex();//should I buffer before, after, or both?
		LastMaintItem++;

		if(LastMaintItem < 0 || LastMaintItem >= IH.NumRows)
			LastMaintItem = 0;
		//remove one deleted item and fix the affected indexes and linked list
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

		while(1)
		{
			if(TempIndItem.NextPart==0)
				break;
			if(TempIndItem.NextPart<0)
			{
				tempsize = 0;
				while(TempIndItem.NextPart<0)
					LoadIndexItem(TempIndItem.NextPart*-1);
			}
			else
				LoadIndexItem(TempIndItem.NextPart);

			tempsize+=TempIndItem.iSize;
		}

		if(tempsize<=0)
			return 0;
		char* TempObj = new char[tempsize];

		LoadIndexItem(a);
		int ind=0;
		while(1)
		{
			if(TempIndItem.NextPart<0)
			{
				while(TempIndItem.NextPart<0)
					LoadIndexItem(TempIndItem.NextPart*-1);
			}

			LargeInt.QuadPart = TempIndItem.Index;
			SetFilePointerEx(dbfile, LargeInt, 0, FILE_BEGIN);
			if(TempIndItem.iSize>0)
			{
				if(ReadFile(dbfile,&TempObj[ind],TempIndItem.iSize,&Written,0)==0)
					break;
			}
			ind+=TempIndItem.iSize;
			if(TempIndItem.NextPart==0)
				break;
			LoadIndexItem(TempIndItem.NextPart);
		}

		//timer1 = GetTickCount() - timer1;
		//cout << "Reading Data took "<<timer1<<" milliseconds.\n";

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

		szRow=0;
		IndexBuffer = 0;
		IndexBackBuffer = 0;
		IndexBuffered = 0;
		NumRowsBuffered = 0;
		NumRowsBackBuffered = 0;
		KeepIndexBuffered = 1;
		LastMaintItem = 0;
		MAXDBIBUFFER = ((1024*1024) * 10) / sizeof(IndexItem);//1MB of RAM

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