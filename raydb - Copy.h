class DBObj;
//I'll make the index a binary tree later (if(Key[1] > Key1 || (Key[1]==Key1 && Key[0]>Key0) )...else....
//the data will be a linked list
//should make sure the first thing in the data is the key and then the size, just incase the index gets broken
#pragma pack(4)
struct IndexItem
{
	unsigned __int64 Key[2];
	__int64 Index;
	__int64 Update;
	int iSize;
	int NextData;
	int PrevData;
	int Left;
	int Right;
	int NextPart;
};

#pragma pack(4)
struct IndexHeader
{
	__int64 CurrUpdate;
	int NumRows;//this includes data fragments
	int NumItems;//this does not
	int DataHead;
	int DataTail;
};

class RayDB
{
public:
	fstream index;
	fstream dbfile;
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
	int LastMaintItem;

	IndexHeader IH;

	IndexItem* IndexBuffer;
	IndexItem* IndexBackBuffer;
	IndexItem TempIndItem;
	int MAXDBIBUFFER;

	void IncrementCurrUpdate()
	{
		IH.CurrUpdate++;
		index.seekp(0);
		index.write((char*)&IH,sizeof(IH));

		//index.seekg(0);
		//index.read((char*)&IH,sizeof(IH));
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
		index.clear();
		index.seekp((a*sizeof(IndexItem)) + sizeof(IH));
		index.write((char*)&TempIndItem,sizeof(IndexItem));
		//index.flush();
	}

	void SaveIndexItemOld(int a)
	{
		if(a < NumRowsBuffered)
		{
			IndexBuffer[a] = TempIndItem;
		}
		else if( (a-NumRowsBuffered) < NumRowsBackBuffered)
		{
			IndexBackBuffer[a-NumRowsBuffered] = TempIndItem;
		}
		//save to file...
		index.clear();
		index.seekp((a*sizeof(IndexItem)) + sizeof(IH));
		index.write((char*)&TempIndItem,sizeof(IndexItem));
		//index.flush();
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
			index.sync();
			index.seekg((NumRowsBuffered*sizeof(IndexItem))+sizeof(IH));
			index.read((char*)&IndexBuffer[NumRowsBuffered], (a+1-NumRowsBuffered)*sizeof(IndexItem));
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

			index.sync();
			index.seekg(sizeof(IH));
			index.read((char*)IndexBuffer, NumRowsBuffered*sizeof(IndexItem));
			IndexBuffered = 1;
			UpdateBuffered = IH.CurrUpdate;

			TempIndItem = IndexBuffer[a];
			return;
		}
		else
		{
			index.seekg((a*sizeof(IndexItem)) + sizeof(IH));
			index.read((char*)&TempIndItem,sizeof(IndexItem));
		}
	}

	void LoadIndexItemOld(int a)
	{
		if(a < NumRowsBuffered && UpdateBuffered == IH.CurrUpdate)
		{
			TempIndItem = IndexBuffer[a];
			return;
		}
		else if( (a-NumRowsBuffered) < NumRowsBackBuffered && UpdateBuffered == IH.CurrUpdate)
		{
			TempIndItem = IndexBackBuffer[a-NumRowsBuffered];
			return;
		}
		else if(a > (NumRowsBuffered + NumRowsBackBuffered)+128 && KeepIndexBuffered>0 && a < MAXDBIBUFFER)
		{
			if(NumRowsBuffered>0  && UpdateBuffered == IH.CurrUpdate)
			{
				index.sync();
				if(NumRowsBackBuffered>0)
				{
					IndexItem* NewBuff;
					IndexItem* OldBuff;
					OldBuff = IndexBuffer;

					int i = NumRowsBuffered;
					NumRowsBuffered = a+1;
					if(NumRowsBuffered> MAXDBIBUFFER)
						NumRowsBuffered = MAXDBIBUFFER;

					NewBuff = new IndexItem[NumRowsBuffered];
					memcpy(NewBuff,OldBuff,i*sizeof(IndexItem));
					memcpy(&NewBuff[i],IndexBackBuffer,NumRowsBackBuffered*sizeof(IndexItem));
					i += NumRowsBackBuffered;
					NumRowsBackBuffered = 0;

					index.seekg((i*sizeof(IndexItem))+sizeof(IH));
					index.clear();
					index.read((char*)&NewBuff[i],sizeof(IndexItem) * (NumRowsBuffered - i));
					IndexBuffer = NewBuff;
					delete[] IndexBackBuffer;
					delete[] OldBuff;
					IndexBackBuffer = OldBuff = 0;

					TempIndItem = IndexBuffer[a];
					return;
				}
				else
				{
					NumRowsBackBuffered = (a+1) - NumRowsBuffered;
					if(NumRowsBackBuffered + NumRowsBuffered > MAXDBIBUFFER)
						NumRowsBackBuffered = MAXDBIBUFFER - NumRowsBuffered;		

					if(NumRowsBackBuffered>1)
					{
						delete[] IndexBackBuffer;

						IndexBackBuffer = new IndexItem[NumRowsBackBuffered];

						index.seekg(((NumRowsBuffered)*sizeof(IndexItem)) + sizeof(IH));
						index.clear();
						index.read((char*)IndexBackBuffer,sizeof(IndexItem)*(NumRowsBackBuffered));
						IndexBuffered = 1;

						//NumRowsBuffered = a+1;
						TempIndItem = IndexBackBuffer[a-NumRowsBuffered];
						return;
					}
				}
			}
			else
			{
				index.sync();
				index.seekg(sizeof(IH));
				NumRowsBackBuffered = 0;
				NumRowsBuffered = a+1;
				if(NumRowsBuffered> MAXDBIBUFFER)
					NumRowsBuffered = MAXDBIBUFFER;

				IndexBuffer = new IndexItem[NumRowsBuffered];
				index.read((char*)IndexBuffer, NumRowsBuffered*sizeof(IndexItem));
				IndexBuffered = 1;
				UpdateBuffered = IH.CurrUpdate;

				TempIndItem = IndexBuffer[a];
				return;
			}
		}
		index.seekg((a*sizeof(IndexItem)) + sizeof(IH));
		index.read((char*)&TempIndItem,sizeof(IndexItem));
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

		if(TempIndItem.Update == IH.CurrUpdate)
		{
			temppos = TempIndItem.Index;
			return child2;
		}
		else
		{
			__int64 tempind = 0;
			int i = TempIndItem.PrevData;

			LoadIndexItem(IH.DataHead);
			if(TempIndItem.Update != IH.CurrUpdate)
			{
				i = IH.DataHead;
				TempIndItem.Update = IH.CurrUpdate;
				SaveIndexItem(IH.DataHead);
			}
			else
			{
				while(i != IH.DataHead)
				{
					if(i==-1)
						return -1;
					LoadIndexItem(i);
					if(TempIndItem.Update == IH.CurrUpdate)
						break;
					i = TempIndItem.PrevData;
				}
			}
			LoadIndexItem(i);
			tempind = TempIndItem.Index + TempIndItem.iSize;
			i = TempIndItem.NextData;
			while(i != child2)
			{
				if(i==-1)
					return -1;
				LoadIndexItem(i);
				TempIndItem.Index = tempind;
				TempIndItem.Update = IH.CurrUpdate;
				tempind += TempIndItem.iSize;
				SaveIndexItem(i);

				i = TempIndItem.NextData;
			}

			LoadIndexItem(i);
			temppos = TempIndItem.Index = tempind;
			TempIndItem.Update = IH.CurrUpdate;
			SaveIndexItem(i);
			return child2;
		}
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
			{//go left
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
			{//go left
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
			{//go left
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

		int slot=0;
		if(IH.NumRows>0)
		{
			slot = FindFreeParent();
			//cout << "FindFreeParent() == "<<slot<<"\n";
			if(slot==-1)
			{
				cout <<"Error! Item already exists!\n";
				return;
			}
			LoadIndexItem(slot);
			if(TempIndItem.Key[0] > Keys[0] || (TempIndItem.Key[0] == Keys[0] && TempIndItem.Key[1] > Keys[1]))
			{
				TempIndItem.Left = IH.NumRows;
				Lefts++;
				//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
			}
			else
			{
				TempIndItem.Right = IH.NumRows;
				Rights++;
				//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
			}
			SaveIndexItem(slot);
		}
		slot = IH.NumRows;

		index.clear();
		IH.CurrUpdate--;
		IncrementCurrUpdate();
		index.seekp(0, ios::end);
		int blanks = ((slot*sizeof(IndexItem))+sizeof(IH))  - index.tellp();
		if(blanks<0)
			blanks = 0;
		blanks /= sizeof(IndexItem);

		if(blanks>0)
		{
			cout << "BLANKS?!?!?\n";
			IndexItem* ptemp=0;
			ptemp = new IndexItem[128];
			memset(ptemp,0,sizeof(IndexItem)*128);
			while(blanks>=128)
			{
				index.write((char*)ptemp, sizeof(IndexItem)*128);
				blanks-=128;
			}
			if(blanks>0)
				index.write((char*)ptemp, sizeof(IndexItem)*blanks);
		}

		dbfile.seekp(0,ios::end);
		TempIndItem.Index = dbfile.tellp();
		TempIndItem.iSize = size;
		TempIndItem.Key[0] = Key0;
		TempIndItem.Key[1] = Key1;
		TempIndItem.Update = IH.CurrUpdate;
		TempIndItem.Left = -1;
		TempIndItem.Right = -1;
		TempIndItem.NextPart = 0;

		if(IH.DataHead == -1)
		{
			IH.DataHead = IH.DataTail = slot;
			TempIndItem.NextData = TempIndItem.PrevData = -1;
			SaveIndexItem(slot);
		}
		else
		{
			TempIndItem.PrevData = IH.DataTail;
			TempIndItem.NextData = -1;
			SaveIndexItem(slot);

			LoadIndexItem(IH.DataTail);
			TempIndItem.NextData = slot;
			SaveIndexItem(IH.DataTail);
			IH.DataTail = slot;
		}
		IH.NumRows++;
		IH.NumItems++;
		//index.flush();
		//index.sync();

		FindIndexItem();

		//dbfile.clear();
		//cout << (char*)Data<<"\n";
		dbfile.write(Data,size);
		//dbfile.flush();
		//dbfile.sync();

		IH.CurrUpdate--;
		IncrementCurrUpdate();
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
				dbfile.seekp(TempIndItem.Index);
				if(size < TempIndItem.iSize + ind)
				{
					int slot;
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
					TempIndItem.NextPart = IH.NumRows * -1;
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
						TempIndItem.Left = IH.NumRows;
						Lefts++;
						//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
					}
					else
					{
						TempIndItem.Right = IH.NumRows;
						Rights++;
						//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
					}
					SaveIndexItem(slot);
					slot = IH.NumRows;
					IH.NumRows++;
					IH.CurrUpdate--;
					IncrementCurrUpdate();

					LoadIndexItem(slot);
					TempIndItem.Key[0] = Keys[0];
					TempIndItem.Key[1] = Keys[1];
					TempIndItem.Update = IH.CurrUpdate;
					TempIndItem.iSize = size - ind;
					TempIndItem.Left = -1;
					TempIndItem.Right = -1;
					TempIndItem.NextData = -1;
					TempIndItem.NextPart = 0;
					TempIndItem.PrevData = IH.DataTail;
					dbfile.seekp(0,ios::end);
					TempIndItem.Index = dbfile.tellp();
					SaveIndexItem(slot);

					LoadIndexItem(IH.DataTail);
					TempIndItem.NextData = slot;
					SaveIndexItem(IH.DataTail);
					IH.DataTail = slot;

					IH.CurrUpdate--;
					IncrementCurrUpdate();

					if(size - ind > 0)
						dbfile.write(&Data[ind],size - ind);
					break;
				}
				else if(TempIndItem.iSize > 0)
					dbfile.write(&Data[ind],TempIndItem.iSize);
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
					TempIndItem.NextPart = IH.NumRows;
					if(TempIndItem.iSize == 0)
						TempIndItem.NextPart *= -1;
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
						TempIndItem.Left = IH.NumRows;
						Lefts++;
						//cout <<"Making "<<IH.NumRows<<" the left child of "<<slot<<".\n";
					}
					else
					{
						TempIndItem.Right = IH.NumRows;
						Rights++;
						//cout <<"Making "<<IH.NumRows<<" the right child of "<<slot<<".\n";
					}
					SaveIndexItem(slot);
					slot = IH.NumRows;
					IH.NumRows++;
					IH.CurrUpdate--;
					IncrementCurrUpdate();

					LoadIndexItem(slot);
					TempIndItem.Key[0] = Keys[0];
					TempIndItem.Key[1] = Keys[1];
					TempIndItem.Update = IH.CurrUpdate;
					TempIndItem.iSize = size - ind;
					TempIndItem.Left = -1;
					TempIndItem.Right = -1;
					TempIndItem.NextData = -1;
					TempIndItem.NextPart = 0;
					TempIndItem.PrevData = IH.DataTail;
					dbfile.seekp(0,ios::end);
					TempIndItem.Index = dbfile.tellp();
					SaveIndexItem(slot);

					LoadIndexItem(IH.DataTail);
					TempIndItem.NextData = slot;
					SaveIndexItem(IH.DataTail);
					IH.DataTail = slot;

					IH.CurrUpdate--;
					IncrementCurrUpdate();

					dbfile.write(&Data[ind],size - ind);
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
			dbfile.seekp(TempIndItem.Index);
			dbfile.write(Data,size);
		}
	}

	void Maintenance()
	{//run this once every time the DB checks the query queue and it is empty
		//buffer...
		BufferIndex();
		LastMaintItem++;

		if(LastMaintItem < 0 || LastMaintItem >= IH.NumRows)
			LastMaintItem = 0;
		//remove one deleted item and fix the affected indexes and linked list
	}

	void BufferIndex()
	{
		//first check to see if it is already buffered..
		if(UpdateBuffered == IH.CurrUpdate && NumRowsBuffered == MAXDBIBUFFER && NumRowsBackBuffered==0)
			return;
		else if(IndexBuffered==1)
		{
			IndexBuffered = 0;
			NumRowsBuffered = 0;
			delete[] IndexBuffer;
			IndexBuffer = 0;
		}
		if(NumRowsBackBuffered>0)
		{
			NumRowsBackBuffered=0;
			delete[] IndexBackBuffer;
			IndexBackBuffer = 0;
		}

		index.sync();
		index.seekg(sizeof(IH));
		NumRowsBuffered = IH.NumRows;
		if(NumRowsBuffered> MAXDBIBUFFER)
			NumRowsBuffered = MAXDBIBUFFER;
		
		IndexBuffer = new IndexItem[NumRowsBuffered];
		index.read((char*)IndexBuffer, NumRowsBuffered*sizeof(IndexItem));
		IndexBuffered = 1;
		UpdateBuffered = IH.CurrUpdate;
	}

	char* GetItem(unsigned __int64 Key0, unsigned __int64 Key1)
	{
		//int timer1 = GetTickCount();
		Keys[0] = Key0;
		Keys[1] = Key1;
		int a;
		a = FindIndexItem();

		//timer1 = GetTickCount() - timer1;
		//cout << "Looking up index took "<<timer1<<" milliseconds.\n";
		//timer1 = GetTickCount();
		if(temppos == -1)
			return 0;

		dbfile.clear();
		//dbfile.sync();
		//dbfile.flush();

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

			dbfile.seekg(TempIndItem.Index);
			if(dbfile.eof())
				break;
			if(TempIndItem.iSize>0)
				dbfile.read(&TempObj[ind],TempIndItem.iSize);
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

		index.open(DBIName,ios::binary | ios::out | ios::in);
		if(!index.is_open())
		{
			cout << "Creating index file "<<DBIName<<".\n";
			index.close();
			index.open(DBIName, ios::binary | ios::out);

			IH.DataHead = IH.DataTail = -1;
			IH.CurrUpdate = 0;
			IH.NumRows = 0;
			IH.NumItems = 0;
			IncrementCurrUpdate();
			index.close();
			index.open(DBIName,ios::binary | ios::out | ios::in);
		}

		dbfile.open(DBName,ios::binary | ios::out | ios::in);
		if(!dbfile.is_open())
		{
			cout << "Creating DB file "<<DBName<<".\n";
			dbfile.close();
			dbfile.open(DBName, ios::binary | ios::out);
			//char c='\0';
			//dbfile.write(&c,1);
			dbfile.close();
			dbfile.clear();
			dbfile.open(DBName,ios::binary | ios::out | ios::in);
		}

		szRow=0;
		IndexBuffer = 0;
		IndexBackBuffer = 0;
		IndexBuffered = 0;
		NumRowsBuffered = 0;
		NumRowsBackBuffered = 0;
		KeepIndexBuffered = 1;
		LastMaintItem = 0;
		MAXDBIBUFFER = 65536*8;

		index.seekg(0);
		index.read((char*)&IH,sizeof(IH));
		index.clear();
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