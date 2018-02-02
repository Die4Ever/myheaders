#pragma once

#define R_NOTHING 0
#define R_IM 1
#define R_OTHER 2

namespace FLAPFrameTypes
{
	const unsigned char SIGNON = 1;
	const unsigned char DATA = 2;
	const unsigned char fERROR = 3;
	const unsigned char SIGNOFF = 4;
	const unsigned char KEEP_ALIVE = 5;
};

#pragma pack(1)
struct FLAP
{
	char Asterisk;
	unsigned char Type;
	unsigned __int16 SeqNum;
	unsigned __int16 DataLen;

	void ToNet()
	{
		SeqNum = htons(SeqNum);
		DataLen = htons(DataLen);
	}

	void ToHost()
	{
		SeqNum = ntohs(SeqNum);
		DataLen = ntohs(DataLen);
	}

	bool Checksum()
	{
		return true;
	}

	int GetLength()
	{
		return ntohs(DataLen)+sizeof(FLAP);
	}
};

#pragma pack(1)
struct TOCSignOnFrame
{
	char Asterisk;
	unsigned char Type;
	unsigned __int16 SeqNum;
	unsigned __int16 DataLen;

	unsigned int FLAPVersion;
	unsigned __int16 TLVTag;
	unsigned __int16 UserNameLen;
	char Username[1];

	void ToNet()
	{
		SeqNum = htons(SeqNum);
		DataLen = htons(DataLen);

		FLAPVersion = htonl(FLAPVersion);
		TLVTag = htons(TLVTag);
		UserNameLen = htons(UserNameLen);
	}

	void ToHost()
	{
		SeqNum = ntohs(SeqNum);
		DataLen = ntohs(DataLen);

		FLAPVersion = ntohl(FLAPVersion);
		TLVTag = ntohs(TLVTag);
		UserNameLen = ntohs(UserNameLen);
	}
};

#pragma pack()

char Hex[17] = "0123456789abcdef";

class Buddy
{
public:
	string sn;
	bool Online;

	Buddy()
	{
		Online=false;
	}

	Buddy(const char *user, bool online) : sn(user), Online(online)
	{
	}

	Buddy(string user, bool online) : sn(user), Online(online)
	{
	}

	unsigned int Hash()
	{
		const char *k = sn.c_str();
		unsigned int h = 0;//*(unsigned int*)k;
		//k+=4;
		for(unsigned int i=1; *k!=0 && i<(103*24)  ;k++,i+=103)
		{
			h += (*k) *i;
		}
		return h;
	}
	virtual void Transport(Buddy *p)
	{
		new(p) Buddy();
		p->sn = sn;
		p->Online=Online;
	}

	static unsigned int Hash(const char *k)
	{
		unsigned int h = 0;//*(unsigned int*)k;
		//k+=4;
		for(unsigned int i=1; *k!=0 && i<(103*24)  ;k++,i+=103)
		{
			h += (*k) *i;
		}
		return h;
	}

	int SortComp(Buddy* objB)
	{
		return sn == objB->sn;
	}

	int SortComp(const char *objB)
	{
		return sn != objB;
	}
};

class TOC2Socket : public PacketizedTCP<FLAP>
{
public:
	unsigned __int16 FLAPSeqClient;
	unsigned __int16 FLAPSeqHost;
	unsigned int LastConnectTime;

	HashTable<Buddy,char*> OfflineBuddies;

	TOC2Socket() : OfflineBuddies(32)
	{
		Prot = TCP;
		FLAPSeqClient = 0;
		FLAPSeqHost = 0;
		LastConnectTime = 0;
	}

	static void Roast(char *dst, char *src, char *roast)
	{
		int l = strlen(roast);
		int i = 0;

		dst[0] = '0';
		dst[1] = 'x';

		for(i=0;src[i];i++)
		{
			char c = src[i] ^ roast[i%l];
			dst[i*2+2] = Hex[(c>>4)];
			dst[i*2+3] = Hex[(c&15)];
		}
		dst[i*2+2] = '\0';
	}

	/*static int calculateCode(char *uid,char *pwd)
	{
		int sn = ((unsigned char)uid[0])-96;
		int pw = ((unsigned char)pwd[0])-96;

		int a = sn * 7696 + 738816;
		int b = sn * 746512;
		int c = pw * a;

		return( c - a + b + 71665152 );
	}*/

	virtual int Connect(char *username, char *password, char *server=/*"aimexpress.oscar.aol.com"*/"toc.oscar.aol.com", int port=29999/*9898*/)
	{
		if(LastConnectTime > (unsigned int)time(0)-120)
			return false;
		LastConnectTime = (unsigned int)time(0);

		SetSync();
		cout << "Connecting....\n";
		int ret = BasicSocket::Connect(server, port);

		if(ret>0)
		{
			cout << "Connection established\n";
			BasicSocket::Send("FLAPON\r\n\r\n", 11, 0);
			BasicSocket::Recv(buffer, MAX_PAC_SIZE, 0);
			//for(int i=0; i<1000 && BasicSocket::Recv(buffer, 16384, 0)<0; i++)
				//RaySleep(1000);
			((TOCSignOnFrame*)buffer)->ToHost();
			if(BasicSocket::iLastRecv>0 && ((TOCSignOnFrame*)buffer)->Asterisk == '*' && ((TOCSignOnFrame*)buffer)->Type == FLAPFrameTypes::SIGNON)
			{
				cout << "Got SIGNON FLAP message\n";

				char *DataBuffer = buffer+sizeof(FLAP);

				((TOCSignOnFrame*)buffer)->Asterisk = '*';
				((TOCSignOnFrame*)buffer)->FLAPVersion = 1;
				((TOCSignOnFrame*)buffer)->SeqNum = 1;
				((TOCSignOnFrame*)buffer)->TLVTag = 1;
				((TOCSignOnFrame*)buffer)->Type = FLAPFrameTypes::SIGNON;
				((TOCSignOnFrame*)buffer)->UserNameLen = strlen(username);
				((TOCSignOnFrame*)buffer)->DataLen = 8+((TOCSignOnFrame*)buffer)->UserNameLen;
				strcpy(((TOCSignOnFrame*)buffer)->Username, username);

				((TOCSignOnFrame*)buffer)->ToNet();
				BasicSocket::Send(buffer, sizeof(TOCSignOnFrame) + strlen(username)-1, 0);
				((TOCSignOnFrame*)buffer)->ToHost();
				//cout << "Last sent == "<<BasicSocket::iLastSent << ", "<< ((TOCSignOnFrame*)buffer)->Username <<"\n";

				((FLAP*)buffer)->Asterisk = '*';
				((FLAP*)buffer)->SeqNum = 2;
				((FLAP*)buffer)->Type = FLAPFrameTypes::DATA;
				
				//sprintf(DataBuffer, "toc2_signon %s %d %s ", server, port, username);
				sprintf(DataBuffer, "toc2_signon login.oscar.aol.com 5190 %s ", username);

				((FLAP*)buffer)->DataLen = strlen(DataBuffer);
				Roast(DataBuffer+((FLAP*)buffer)->DataLen, password, "Tic/Toc");
				((FLAP*)buffer)->DataLen += strlen(DataBuffer+((FLAP*)buffer)->DataLen);

				sprintf(DataBuffer+((FLAP*)buffer)->DataLen, " english \"TIC:TOC2:REVISION\" 160 %u"
					//, calculateCode(username, password) );
					, (unsigned int)( ((unsigned int)username[0]) * ((unsigned int)password[0]) * 7696 ));

				((FLAP*)buffer)->DataLen += strlen(DataBuffer+((FLAP*)buffer)->DataLen)+1;

				//cout << DataBuffer << "\n";

				//RaySleep(1000);
				((FLAP*)buffer)->ToNet();
				BasicSocket::Send(buffer, ntohs(((FLAP*)buffer)->DataLen)+sizeof(FLAP), 0);
				((FLAP*)buffer)->ToHost();

				//cout << "\nLast sent == "<<BasicSocket::iLastSent << "\n";

				SetAsync();
				BasicSocket::iLastRecv=0;
				for(unsigned int i=0;i<1000 && BasicSocket::iLastRecv<1;i++)
				{
					BasicSocket::Recv(buffer+1024, MAX_PAC_SIZE-1024, 0);
					RaySleep(1000);
				}
				cout << "Last recv == "<<BasicSocket::iLastRecv << "-\n"<<DataBuffer+1024<<"\n-\n";

				strcpy(DataBuffer, "toc_init_done");
				((FLAP*)buffer)->DataLen = 14;
				((FLAP*)buffer)->SeqNum = 3;
				((FLAP*)buffer)->ToNet();
				BasicSocket::Send(buffer, ntohs(((FLAP*)buffer)->DataLen)+sizeof(FLAP), 0);
				((FLAP*)buffer)->ToHost();
				//cout << "Last sent == "<<BasicSocket::iLastSent << "\n";
				FLAPSeqClient = 4;

				strcpy(DataBuffer, "toc2_set_pdmode 1");
				((FLAP*)buffer)->DataLen = strlen(DataBuffer)+1;
				((FLAP*)buffer)->SeqNum = FLAPSeqClient++;
				((FLAP*)buffer)->ToNet();
				BasicSocket::Send(buffer, ntohs(((FLAP*)buffer)->DataLen)+sizeof(FLAP), 0);
				((FLAP*)buffer)->ToHost();
				//cout << "Last sent == "<<BasicSocket::iLastSent << "\n";

				char user[128];
				char im[4096];
				bool baway;

				//wait on buddylist update, otherwise I'm not really online
				BasicSocket::iLastRecv=0;
				for(unsigned int i=0;i<1000 && BasicSocket::iLastRecv<1; i++)
				{
					RecvIM(user, im, baway);
					RaySleep(1000);
				}

				//SetAsync();

				if(BasicSocket::iLastRecv>0)
				{
					cout << "Connected!\n";
					return true;
				}
			}
		}

		LastConnectTime = (unsigned int)time(0)+60;
		BasicSocket::Status = BasicSocket::Closed;
		cout << "Connection Failed!\n";
		return false;
	}

	bool IsOnline(const char *user)
	{
		Buddy *b=OfflineBuddies.Get((char*)user);
		if(!b)
			return 1;
		return b->Online;
	}

	virtual int UpdateBuddy(const char *user)
	{
		char buff[16384];
		char *DataBuffer=buff+sizeof(FLAP);

		sprintf(DataBuffer, "toc_get_status \"%s\"", user);
		((FLAP*)buff)->Asterisk = '*';
		((FLAP*)buff)->DataLen = strlen(DataBuffer)+1;
		((FLAP*)buff)->SeqNum = FLAPSeqClient++;
		((FLAP*)buff)->Type = FLAPFrameTypes::DATA;
		((FLAP*)buff)->ToNet();
		BasicSocket::Send(buff, ntohs(((FLAP*)buff)->DataLen)+sizeof(FLAP), 0);

		if(BasicSocket::iLastSent<1)
			Status = Closed;

#ifdef _DEBUG
		cout << "Last sent == "<<BasicSocket::iLastSent << "\n";
#endif
		return BasicSocket::iLastSent;
	}

	//need to check if the user is online first
	virtual int SendIM(const char *user, const char *im)
	{
		if(!IsOnline(user))
		{
#ifdef _DEBUG
			cout << user<<" is offline.\n";
#endif
			return 0;
		}

#ifdef OUTPUT_IMS
		cout << "To "<<user<<": "<<im<<"\n";
#endif
		char buff[16384];
		char *DataBuffer=buff+sizeof(FLAP);

		sprintf(DataBuffer, "toc2_send_im \"%s\" \"%s\"", user, im);
		((FLAP*)buff)->Asterisk = '*';
		((FLAP*)buff)->DataLen = strlen(DataBuffer)+1;
		((FLAP*)buff)->SeqNum = FLAPSeqClient++;
		((FLAP*)buff)->Type = FLAPFrameTypes::DATA;
		((FLAP*)buff)->ToNet();
		BasicSocket::Send(buff, ntohs(((FLAP*)buff)->DataLen)+sizeof(FLAP), 0);

		if(BasicSocket::iLastSent<1)
			Status = Closed;

#ifdef _DEBUG
		cout << "Last sent == "<<BasicSocket::iLastSent << "\n";
#endif
		return BasicSocket::iLastSent;
	}

	//need to check if the user is online first
	virtual int SendSplitIM(const char *user, const char *im)
	{
		if(!IsOnline(user))
		{
#ifdef _DEBUG
			cout << user<<" is offline.\n";
#endif
			return 0;
		}

		if(strlen(im)<2048)
			return SendIM(user, im);

		char buff[16384];
		char *s = (char*)im;
		char *e = NULL;
		char *e2 = NULL;
		e=strchr(s, '\n');
		while(e!=NULL && ((unsigned int)(e-s))<2048)
		{
			e2=strchr(e+1, '\n');
			if(e2 && ((unsigned int)(e2-s))<2048)
				e=e2;
			else if(e2 || strlen(s)>2048 )
			{
#ifdef OUTPUT_IMS
				printf("To %s: ",user);
				fwrite(s, 1, e-s, stdout);
				printf("\n");
#endif
				char *DataBuffer=buff+sizeof(FLAP);

				int i = sprintf(DataBuffer, "toc2_send_im \"%s\" \"", user);
				memcpy(DataBuffer+i, s, e-s);
				i+= ((int)(e-s));
				DataBuffer[i++] = '\"';
				DataBuffer[i] = '\0';

				s=e+1;
				e=strchr(s, '\n');

				((FLAP*)buff)->Asterisk = '*';
				((FLAP*)buff)->DataLen = strlen(DataBuffer)+1;
				((FLAP*)buff)->SeqNum = FLAPSeqClient++;
				((FLAP*)buff)->Type = FLAPFrameTypes::DATA;
				((FLAP*)buff)->ToNet();
				BasicSocket::Send(buff, ntohs(((FLAP*)buff)->DataLen)+sizeof(FLAP), 0);

				if(BasicSocket::iLastSent<1)
					Status = Closed;

#ifdef _DEBUG
				cout << "Last sent == "<<BasicSocket::iLastSent << "\n";
#endif
			}
			else
				break;
		}

		//cout << "strlen(s)=="<<strlen(s) <<"\n";
		SendIM(user, s);

		return BasicSocket::iLastSent;
	}

	virtual int RecvIM(char *user, char *im, bool &away)
	{
		char *buff;
		if( (buff = Recv())!=NULL && ((FLAP*)buff)->Asterisk == '*' && ((FLAP*)buff)->Type == FLAPFrameTypes::DATA )
		{
			char *DataBuffer = buff+sizeof(FLAP);
			//cout << "Received-\n"<<DataBuffer<<"\n-\n";
			((FLAP*)buff)->ToHost();

#ifdef OUTPUT_IMS_DATA
			fwrite(DataBuffer, 1, ((FLAP*)buff)->DataLen, stdout);
			fwrite("\n", 1, 1, stdout);
#endif

			if( memcmp(DataBuffer, "UPDATE_BUDDY2:", 14)==0 )
			{
				char *sn = DataBuffer+14;
				char *e = strchr(DataBuffer+14, ':');
				if(e)
				{
					*e='\0';
					e++;
					if(*e=='T')
					{
						fwrite(sn, 1, e-sn-1, stdout);
						printf(" is online\n");

						Buddy *b=OfflineBuddies.Get(sn);
						if(!b)
							OfflineBuddies.Create( Buddy(sn,1) );
						else
							b->Online=1;
					}
					else
					{
						fwrite(sn, 1, e-sn-1, stdout);
						printf(" is offline\n");

						Buddy *b=OfflineBuddies.Get(sn);
						if(!b)
							OfflineBuddies.Create( Buddy(sn,0) );
						else
							b->Online=0;
					}
				}
			}
			else if( memcmp(DataBuffer, "IM_IN2:", 7)==0 )
			{
				char *e = strchr(DataBuffer+7, ':');
				if(e)
				{
					*e = '\0';
					strcpy(user, DataBuffer+7);

					away=0;
					if( *(e+1) == 'T' || *(e+1) == 't' )
						away = 1;
					e+=5;
					int len = ((FLAP*)buff)->DataLen-(e-DataBuffer);
					if(len)
					{
						int a = 0;
						bool html = 0;
						for(int i=0;i<len;i++,a++)
						{
							im[a] = e[i];
							if(im[a]=='<')
								html=1;
							else if(im[a]=='>')
							{
								a--;
								html=0;
							}
							if(html==1)
								a--;
							else if(e[i] == '&')
							{
								int slen = strlen(e+i);
								if(slen >= 5 && memcmp(e+i, "&amp;", 5)==0)
									i+=4;
								else if(slen >= 6 && memcmp(e+i, "&quot;", 6)==0)
								{
									im[a] = '"';
									i+=5;
								}
							}
						}
						im[a] = '\0';
					}

#ifdef OUTPUT_IMS
					if(away)
						cout << "Auto-response from "<<user<<": "<<im<<"\n";
					else
						cout << user<<": "<<im<<"\n";
#endif

					return R_IM;
				}
			}
			return R_OTHER;
		}
		return R_NOTHING;
	}
};
