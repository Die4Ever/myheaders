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
		return sn != objB->sn;
	}

	int SortComp(const char *objB)
	{
		return sn != objB;
	}
};

class OSCARSocket : public PacketizedTCP<FLAP>
{
public:
	unsigned __int16 FLAPSeqClient;
	unsigned __int16 FLAPSeqHost;
	unsigned int LastConnectTime;

	HashTable<Buddy,char*> OfflineBuddies;

	OSCARSocket() : OfflineBuddies(32)
	{
		Prot = TCP;
		FLAPSeqClient = 0;
		FLAPSeqHost = 0;
		LastConnectTime = 0;
	}
};
