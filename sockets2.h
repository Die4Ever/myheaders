#include <winsock2.h>
#ifdef PHONE
#include <ws2tcpip.h>
#endif

//split packets up into 4 KB segments that are then put together when completed
const int cPacketTypes = 20;

#pragma pack(4)
struct sPacketHeader
{
	int		Part;
	int		PartSize;
	int		iLength;
	int		iType;//-666 is bad
	unsigned int		iID;
	int		iChecksum;
};

class sPacket : public ContObj<sPacket>
{
public:
	sPacketHeader pHeader;
	char*	pBuffer;
	int		recved;//-1 for outgoing, # of bytes recved

	sPacket()
	{
		pBuffer=0;
	}
	virtual ~sPacket()
	{
		if(pBuffer!=0)
			delete[] pBuffer;
	}
};

class SocketObject : public ContObj<SocketObject>
{
public:
	static int iWinSockInitialized;
	static int numSocketObjects;

	int MaxPartSize;
	int iPort;
	SOCKET skSocket;
	int SocketID;
	int BytesRet;
	int iLastSent,iLastRecv;
	int iSent, iRecv;
	int iLastUsed;
	unsigned int pacID;
	int	iPartialData;
	char* cPartialData;
	char cPartialData1[4096];
	char cPartialData2[4096];

	LinkedList<sPacket> pac;
	sPacket TempPac;
	//int iStatus;
	enum SStatuses
	{
		Created,
		Closed,
		Not_Responding,
		Closing,
		Connected
	};
	SStatuses Status;

	void Disconnect();
	int Bind(int iport);
	int Listen();
	int Connect(char* szServerAddress, int iport);
	int Accept(SocketObject &skAcceptSocket);
	int Send(char *szBuffer, int iBufLen, int iFlags);
	int Recv(char *szBuffer, int iBufLen, int iFlags);
	int SendQ(char *szBuffer, int iBufLen, int iFlags);
	int ProcPacket(char* szBuffer, int iBufLen);

	int SendPack(char *Data, int iBufLen, int iFlags, int type);
	int RecvPack(int iFlags=0);

	int ErrorCheck();

	class SendQueue
	{
	public:
		int		iType;
		char*	data;
		int		size;
		int		flags;
		SendQueue* pNext;

		SendQueue()
		{
			pNext=NULL;
			data=NULL;
		}
	};
	SendQueue* pQHead;SendQueue* pQTail;SendQueue* pQTemp;

	void vAddQueue(char* szBuffer,int iBufLen,int iFlags, int type);
	int iSendQueue();
	void ClearQueue();

	unsigned int iDownloadID;//0 is no download
	int		TypesQueued[cPacketTypes];

	SocketObject();
	~SocketObject();
};
int SocketObject::iWinSockInitialized = 0;
int SocketObject::numSocketObjects = 0;

SocketObject::SocketObject()
{
	cPartialData = cPartialData1;
	MaxPartSize = 1024 * 32;
	iPartialData=0;

	memset(TypesQueued,0,4*cPacketTypes);
	pQHead=pQTail=pQTemp=NULL;

	SocketID = numSocketObjects;
	numSocketObjects++;
	pacID = iLastSent = iLastRecv = iSent = iRecv = 0;

	int a;
	if(iWinSockInitialized==0)
	{
		WSADATA	wsaData;
		WORD	wVersionRequested;

		wVersionRequested = MAKEWORD(2,2);
		skSocket = INVALID_SOCKET;
		a=WSAStartup(wVersionRequested, &wsaData);
	}
	//iStatus = 0;
	Status = this->Created;

	iWinSockInitialized = 1;
}

SocketObject::~SocketObject()
{
	Disconnect();
}

void SocketObject::Disconnect()
{
	if(skSocket != INVALID_SOCKET)
	{
		closesocket(skSocket);
		skSocket = INVALID_SOCKET;
	}
	Status = this->Closed;
}

int SocketObject::Bind(int iport)
{
	sockaddr_in saServerAddress;
	skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(skSocket == INVALID_SOCKET)
		return false;
	
	if(iport == 0)
		iport = this->iPort;
	else
		this->iPort = iport;

	memset(&saServerAddress, 0, sizeof(sockaddr_in));
	saServerAddress.sin_family = AF_INET;
	saServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	saServerAddress.sin_port = htons(iPort);
	if( bind(skSocket, (sockaddr*) &saServerAddress, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		ErrorCheck();
		Disconnect();
		return false;
	}
	else
	{
		u_long iMode = 1;
	ioctlsocket(skSocket, FIONBIO, &iMode);
		return true;
	}
}

int SocketObject::Listen()
{
	int i;
	i = listen( skSocket, 64);
	if(i == -1)
		ErrorCheck();
	return i;
}

int SocketObject::Accept(SocketObject &skAcceptSocket)
{
	sockaddr_in		saClientAddress;
	int				iClientSize = sizeof(sockaddr_in);

	skAcceptSocket.skSocket = accept( skSocket, (struct sockaddr*)&saClientAddress, &iClientSize );

	if(skAcceptSocket.skSocket == INVALID_SOCKET )
	{
		ErrorCheck();
		return false;
	}
	else
	{
		skAcceptSocket.Status = this->Connected;
		return true;
	}
}

int SocketObject::Connect(char *szServerAddress, int iport)
{
	struct sockaddr_in	serv_addr;
	LPHOSTENT			lpHost;
	int					err;
	if(iport==0)
		iport = this->iPort;
	else
		this->iPort = iport;

	Disconnect();
	//Open the socket
	skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(skSocket == INVALID_SOCKET)
		return false;
	memset(&serv_addr,0,sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(szServerAddress);
	
	if(serv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		lpHost = gethostbyname(szServerAddress);
		if(lpHost != NULL)
		{
			serv_addr.sin_addr.s_addr = ((LPIN_ADDR)lpHost->h_addr_list)->s_addr;
		}
		else
			return false;
	}

	//Assign the port
	serv_addr.sin_port = htons(iPort);
	//Establish the connection
	err = connect(skSocket, (struct sockaddr*)&serv_addr, sizeof(sockaddr));
	if(err == SOCKET_ERROR)
	{
		int a = GetLastError();
		Disconnect();
		return false;
	}
	Status = this->Connected;
	u_long iMode = 1;
	ioctlsocket(skSocket,FIONBIO,&iMode);
	return true;
}

int SocketObject::Send(char *szBuffer, int iBufLen, int iFlags)
{
	iLastSent = send(skSocket, szBuffer, iBufLen, iFlags);
	iSent += iLastSent;
	return iLastSent;
}

int SocketObject::Recv(char *szBuffer, int iBufLen, int iFlags)
{
	if(iFlags==MSG_PEEK)
	{
		iLastRecv = recv(skSocket, szBuffer, iBufLen, iFlags);
		return iLastRecv;
	}
	iLastRecv = recv(skSocket, szBuffer, iBufLen, iFlags);
	iRecv += iLastRecv;
	return iLastRecv;
}

int SocketObject::iSendQueue()
{
	if(pQHead == NULL)
		return 0;
	TypesQueued[pQHead->iType]--;

	SendQ(pQHead->data,pQHead->size,pQHead->flags);
	pQTemp = pQHead;
	if(pQTail==pQHead)
		pQTail=NULL;
	pQHead = pQHead->pNext;
	delete pQTemp;
	pQTemp = NULL;
	return iLastSent;
}

void SocketObject::vAddQueue(char* szBuffer,int iBufLen,int iFlags, int type)
{
	if(pQHead==NULL)
	{
		pQHead = pQTail = new SendQueue;
	}
	else
	{
		pQTail->pNext = new SendQueue;
		pQTail = pQTail->pNext;
	}
	pQTail->flags = iFlags;
	pQTail->iType = type;
	TypesQueued[type]++;
	pQTail->size = iBufLen;
	pQTail->data = new char[pQTail->size];
	memcpy(pQTail->data,szBuffer, iBufLen);
	return;
}

void SocketObject::ClearQueue()
{
	while(1)
	{
		if(pQHead == NULL)
			return;
		TypesQueued[pQHead->iType]--;

		pQTemp = pQHead;
		if(pQTail==pQHead)
			pQTail=NULL;
		pQHead = pQHead->pNext;
		delete pQTemp;
		pQTemp = NULL;
	}
}

int SocketObject::SendQ(char *szBuffer, int iBufLen, int iFlags)
{
	iLastSent = send(skSocket, szBuffer, iBufLen, iFlags);
	iSent += iLastSent;
	if(iLastSent<1)
	{
		vAddQueue(szBuffer, iBufLen,iFlags,0);//queue it up
	}
	else if(iLastSent<iBufLen)
	{
		//ReportError("\n---------------Error: Partial packet sent!---------------\n",3);
	}
	return iLastSent;
}

int SocketObject::SendPack(char *Data, int iBufLen, int iFlags, int type)
{
	int parts=0;
	int iBufLen2 = iBufLen;
	parts = iBufLen / this->MaxPartSize + 1;
	iBufLen2 += sizeof(sPacketHeader) * parts;

	pac.pTemp = new sPacket;
	pac.Create(pac.pTemp);

	pac.pTemp->recved = -1;
	pac.pTemp->pBuffer = Data;
	pac.pTemp->pHeader.iID = pacID;
	pac.pTemp->pHeader.iLength = iBufLen;
	pac.pTemp->pHeader.iType = type;
	pac.pTemp->pHeader.iChecksum = pac.pTemp->pHeader.iID * type + iBufLen;

	pac.pTemp->pBuffer = new char[iBufLen2];
	int i=0;
	int a=0;

	for(pac.pTemp->pHeader.Part = 0; pac.pTemp->pHeader.Part < parts; pac.pTemp->pHeader.Part++)
	{
		if( MaxPartSize <= iBufLen - (pac.pTemp->pHeader.Part*MaxPartSize))
			a = MaxPartSize;
		else
			a = iBufLen - (pac.pTemp->pHeader.Part*MaxPartSize);
		pac.pTemp->pHeader.PartSize = a;
		if(a==0)
			break;

		memcpy( &pac.pTemp->pBuffer[i], &pac.pTemp->pHeader, sizeof(sPacketHeader));
		i+=sizeof(sPacketHeader);
		memcpy( &pac.pTemp->pBuffer[i], &Data[pac.pTemp->pHeader.Part*MaxPartSize], a );
		i+=a;
		a+=sizeof(sPacketHeader);

		vAddQueue(&pac.pTemp->pBuffer[i-a] , a,iFlags,pac.pTemp->pHeader.iType);//queue it up
		if(i >= iBufLen2)
			break;
	}
	pacID++;

	delete pac.pTemp;

	iSendQueue();//send the first item in the queue

	//check for errors....
	if(iLastSent == -1)
		ErrorCheck();

	return iLastSent;
}

int SocketObject::ProcPacket(char* szBuffer, int iBufLen)
{
	if(sizeof(TempPac.pHeader) > iBufLen)
	{
		return -1;
	}
	else
	{
		memcpy(&TempPac.pHeader, szBuffer, sizeof(TempPac.pHeader));
		if(TempPac.pHeader.PartSize + sizeof(TempPac.pHeader) > iBufLen)
		{
			return -1;
		}
		for(pac.pTemp=pac.pHead; pac.pTemp != NULL && pac.pTemp->pHeader.iID != TempPac.pHeader.iID; pac.pTemp = pac.pTemp->pNext);
		if(pac.pTemp == NULL)
		{
			pac.pTemp = new sPacket;
			pac.Create(pac.pTemp);
			pac.pTemp->pHeader = TempPac.pHeader;
			pac.pTemp->recved = 0;
			pac.pTemp->pBuffer = new char[TempPac.pHeader.iLength];
		}
		pac.pTemp->pHeader = TempPac.pHeader;
		memcpy(&pac.pTemp->pBuffer[TempPac.pHeader.Part*MaxPartSize], &szBuffer[sizeof(TempPac.pHeader)],TempPac.pHeader.PartSize);
		pac.pTemp->recved += TempPac.pHeader.PartSize;

		return pac.pTemp->pHeader.PartSize+sizeof(TempPac.pHeader);
	}
	return 0;
}

int SocketObject::RecvPack(int iFlags)
{
	int i=0;
	BytesRet=0;
	Recv(&cPartialData[iPartialData], 65536, iFlags);
	if(iLastRecv == -1)
	{
		ErrorCheck();
		return -1;
	}
	while(1)
	{
		i = ProcPacket(&cPartialData[BytesRet], iPartialData+iLastRecv-BytesRet);
		if(i<0)
		{
			if(cPartialData == cPartialData1)
			{
				memcpy(cPartialData2, &cPartialData1[BytesRet],iPartialData+iLastRecv-BytesRet);
				iPartialData = iPartialData+iLastRecv-BytesRet;
				cPartialData = cPartialData2;
			}
			else
			{
				memcpy(cPartialData1, &cPartialData2[BytesRet],iPartialData+iLastRecv-BytesRet);
				iPartialData = iPartialData+iLastRecv-BytesRet;
				cPartialData = cPartialData1;
			}
			return BytesRet;
		}
		BytesRet += i;
	}
	return BytesRet;
}

int SocketObject::ErrorCheck()
{
	int i;
	char Error[2048];
	i = WSAGetLastError();

	/*switch(i)
	{
	case WSAEINTR:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Function call interrupted!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEACCES:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Permission denied!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEFAULT:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Bad address!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEINVAL:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Invalid argument!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEMFILE:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Too many open sockets!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEWOULDBLOCK:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Would block!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error, 0);break;
	case WSAEINPROGRESS:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, WSAEINPROGRESS!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEALREADY:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, WSAEALREADY!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAENOTSOCK:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Socket operation on nonsocket!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEMSGSIZE:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Datagram message too long!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEADDRINUSE:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Address already in use!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAENETDOWN:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Network is down!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAENETUNREACH:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Network is unreachable!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAENETRESET:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Network dropped connection on reset!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAECONNABORTED:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Software caused connection abort!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAECONNRESET:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Connection reset by peer!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAENOBUFS:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, No buffer space available!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAESHUTDOWN:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Cannot send after socket shutdown!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAETOOMANYREFS:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Too many references to some kernel object!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAETIMEDOUT:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Connection timed out!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAECONNREFUSED:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Connection refused!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAEHOSTDOWN:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Host down!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAEHOSTUNREACH:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, No route to host!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAEPROCLIM:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Too many processes using WinSock!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSASYSNOTREADY:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Network subsystem is unavailable!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAVERNOTSUPPORTED:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, A newer version of WinSock is required!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;
	case WSAEDISCON:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Graceful shutdown in progress!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAEREFUSED:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Database query refused!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	case WSAHOST_NOT_FOUND:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Host not found!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);
		this->Disconnect();break;

	default:
		sprintf_s(Error, 2048, "SocketID = %d, PacketType = %d,Sockets Open = %d,Error Number = %d, Unknown error!",this->SocketID,this->TempPac.pHeader.iID,this->numSocketObjects, i);
		ReportError(Error);break;
	}*/
	return 0;
}