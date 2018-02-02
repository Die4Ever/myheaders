#pragma once

#ifdef WIN32
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#endif
#ifdef PHONE
#include <ws2tcpip.h>
#endif
//need to make WIN32 sockets restore their sync/async state when reconnecting
#ifdef WIN32

class BasicSocket
{
public:
	static int SocketsInited;

	int iPort;
	//int Prot;
	SOCKET skSocket;
	int iLastSent,iLastRecv;
	int iSent, iRecv;
	unsigned int iLastUsed;

	sockaddr_in saAddress;
	sockaddr_in saRemoteAddress;

	enum SStatuses
	{
		Created,
		Closed,
		Not_Responding,
		Closing,
		Connected
	};

	enum Protocols
	{
		TCP,UDP,
	};
	Protocols Prot;

	volatile SStatuses Status;

	BasicSocket()
	{
		if(SocketsInited == 0)
		{
			WSADATA	wsaData;
			WORD	wVersionRequested;

			wVersionRequested = MAKEWORD(2,2);
			skSocket = INVALID_SOCKET;
			WSAStartup(wVersionRequested, &wsaData);
			SocketsInited = 1;
		}
		memset(&saAddress, 0, sizeof(sockaddr_in));
		memset(&saRemoteAddress, 0, sizeof(sockaddr_in));
		iPort = iLastSent = iLastRecv = iSent = iRecv = 0;
		Prot = TCP;
		Status = this->Created;
		skSocket = INVALID_SOCKET;

		iLastUsed = (unsigned int)time(0);
	}

	BasicSocket(BasicSocket &s)
	{
		saAddress=s.saAddress;
		saRemoteAddress=s.saRemoteAddress;
		iPort=s.iPort;
		iLastSent=s.iLastSent;
		iLastRecv=s.iLastRecv;
		iLastUsed = (unsigned int)time(0);
		this->iRecv=s.iRecv;
		iSent=s.iSent;
		Prot=s.Prot;
		skSocket=s.skSocket;
		Status=s.Status;
		s.skSocket=INVALID_SOCKET;
		s.Status=Closed;
	}

	~BasicSocket()
	{
		Disconnect();
	}

	void Disconnect()
	{
		if(skSocket != INVALID_SOCKET)
		{
			closesocket(skSocket);
			skSocket = INVALID_SOCKET;
		}
		Status = this->Closed;
	}

	void SetReuse()
	{
		//SO_REUSEADDR
		u_long iMode = 1;
		//ioctlsocket(skSocket, SO_REUSEADDR, &iMode);
		setsockopt(skSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iMode, sizeof(iMode));
	}

	int Bind(int iport)
	{

		if(Prot == TCP)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else if(Prot == UDP)
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		else
			return 0;

		if(skSocket == INVALID_SOCKET)
			return false;

		if(iport == 0)
			iport = this->iPort;
		else
			this->iPort = iport;

		SetReuse();

		memset(&saAddress, 0, sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		saAddress.sin_port = htons((u_short)iPort);
		if( ::bind(skSocket, (sockaddr*) &saAddress, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			//ErrorCheck();
			Disconnect();
			return false;
		}
		else
		{
			iLastUsed = (unsigned int)time(0);
			this->Status = this->Connected;
			u_long iMode = 1;
			ioctlsocket(skSocket, FIONBIO, &iMode);
			return true;
		}
	}

	int Listen()
	{
		return listen( skSocket, 64);
	}

	int Accept(BasicSocket &skAcceptSocket)
	{
		int				iClientSize = sizeof(sockaddr_in);

		skAcceptSocket.skSocket = accept( skSocket, (struct sockaddr*)&skAcceptSocket.saAddress, &iClientSize );

		if(skAcceptSocket.skSocket == INVALID_SOCKET )
		{
			return false;
		}
		else
		{
			skAcceptSocket.Prot = Prot;
			iLastUsed = (unsigned int)time(0);
			skAcceptSocket.iLastUsed = (unsigned int)time(0);
			//u_long iMode = 1;
			//ioctlsocket(skSocket, FIONBIO, &iMode);
			skAcceptSocket.Status = this->Connected;
			return true;
		}
	}

	void SetRemoteAddr(char *szServerAddress, int iport)
	{
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saRemoteAddress.sin_addr.s_addr == INADDR_NONE)
			memcpy(&saRemoteAddress.sin_addr.S_un, gethostbyname(szServerAddress)->h_addr, 4);

		//Assign the port
		saRemoteAddress.sin_port = htons(iport);
	}

	int Connect(const char *szServerAddress, int iport)
	{
		//struct sockaddr_in	serv_addr;
		//LPHOSTENT			lpHost;
		int					err;
		if(iport==0)
			iport = this->iPort;
		else
			this->iPort = iport;

		Disconnect();
		//Open the socket
		if(Prot == TCP)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else if(Prot == UDP)
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		else return 0;

		if(skSocket == INVALID_SOCKET)
			return false;

		//if(Prot == 0)
		//	Bind(iPort);

		memset(&saAddress,0,sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saAddress.sin_addr.s_addr == INADDR_NONE)
		{
			hostent *host = gethostbyname(szServerAddress);
			if(host)
				memcpy(&saAddress.sin_addr.S_un, host->h_addr, 4);
			else
			{
				Disconnect();
				return false;
			}
		}

		//Assign the port
		saAddress.sin_port = htons(iPort);

		//SetRemoteAddr(szServerAddress, iPort);
		saRemoteAddress = saAddress;
		//Establish the connection
		if(Prot == UDP)
		{
			iLastUsed = (unsigned int)time(0);
			u_long iMode = 1;
			ioctlsocket(skSocket, FIONBIO, &iMode);
			Status = this->Connected;
			return 1;
		}

		err = connect(skSocket, (struct sockaddr*)&saAddress, sizeof(sockaddr));
		if(err == SOCKET_ERROR)
		{
			Disconnect();
			return false;
		}
		iLastUsed = (unsigned int)time(0);
		//u_long iMode = 1;
		//ioctlsocket(skSocket, FIONBIO, &iMode);
		Status = this->Connected;
		return true;
	}

	void SetAsync()
	{
		u_long iMode = 1;
		ioctlsocket(skSocket, FIONBIO, &iMode);
	}

	void SetSync()
	{
		u_long iMode = 0;
		ioctlsocket(skSocket, FIONBIO, &iMode);
	}

	int Send(const char *szBuffer, int iBufLen, int iFlags=0)
	{
		//cout << "Sending - \n"<<szBuffer<<"\n\n";
		if(Prot == TCP)//TCP
		{
			iLastSent = send(skSocket, szBuffer, iBufLen, iFlags);
			if(iLastSent>0)
			{
				iLastUsed = (unsigned int)time(0);
				//fwrite(szBuffer, 1, iLastSent, stdout);
			}
			else if(iLastSent == SOCKET_ERROR)
				ErrorCheck();
			return iLastSent;
		}
		else if(Prot == UDP)//UDP
		{
			iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastSent == SOCKET_ERROR)
				ErrorCheck();
			return iLastSent;
		}
		else
			return -1;
	}

	int Recv(char *szBuffer, int iBufLen, int iFlags=0)
	{
		if(Prot == TCP)//TCP
		{
			iLastRecv = recv(skSocket, szBuffer, iBufLen, iFlags);
			if(iLastRecv>0)
			{
				iLastUsed = (unsigned int)time(0);
				//fwrite(szBuffer, 1, iLastRecv, stdout);
			}
			else if(iLastRecv == 0)
				Status = Closed;
			else if(iLastRecv == SOCKET_ERROR)
				ErrorCheck();
			return iLastRecv;
		}
		else if(Prot == UDP)//UDP
		{
			//int len = sizeof(saRemoteAddress);
			iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, iFlags, 0, 0);
			if(iLastRecv > 0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastRecv == 0)
				Status = Closed;
			else if(iLastRecv == SOCKET_ERROR)
				ErrorCheck();
			return iLastRecv;
		}
		else
			return -1;
	}

	int SendTo(const char *szBuffer, int iBufLen, char *Address, int port, int iFlags=0)
	{
		iPort = port;
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(Address);
		saRemoteAddress.sin_port = htons(iPort);

		iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastSent == SOCKET_ERROR)
			ErrorCheck();
		return iLastSent;
	}

	int SendTo(const char *szBuffer, int iBufLen, sockaddr_in &address, int iFlags=0)
	{
		saRemoteAddress = address;

		iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastSent == SOCKET_ERROR)
			ErrorCheck();
		return iLastSent;
	}

	int RecvFrom(char *szBuffer, int iBufLen, int iFlags=0)
	{
		int a = sizeof(saRemoteAddress);
		iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), &a);
		if(iLastRecv > 0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastRecv == 0)
				Status = Closed;
		else if(iLastRecv == SOCKET_ERROR)
			ErrorCheck();

		return iLastRecv;
	}

	void ErrorCheck()
	{
		if(Prot == TCP)
		{
			int lasterror = WSAGetLastError();
			switch(lasterror)
			{
			case WSAENETDOWN:
			case WSAENOTCONN:
			case WSAENETRESET:
			case WSAENOTSOCK:
			case WSAESHUTDOWN:
			case WSAECONNABORTED:
			case WSAETIMEDOUT:
			case WSAECONNRESET:
				Status = Closed;
				break;
			case WSAEWOULDBLOCK:
				break;
			default:
#ifdef _IOSTREAM_
				cout << "some other error!\n";
#endif
				break;
			}
		}
	}
};
#ifdef RAYHEADERSMAIN
int BasicSocket::SocketsInited = 0;
#endif

#else

class BasicSocket
{
public:
	static int SocketsInited;

	int iPort;

	enum Protocols
	{
		TCP,UDP
	};
	Protocols Prot;

	int ASYNC;
	int skSocket;
	int iLastSent,iLastRecv;
	int iSent, iRecv;
	unsigned int iLastUsed;

	sockaddr_in saAddress;
	sockaddr_in saRemoteAddress;

	enum SStatuses
	{
		Created,
		Closed,
		Not_Responding,
		Closing,
		Connected
	};
	volatile SStatuses Status;

	void SetAsync()
	{
		ASYNC = MSG_DONTWAIT;
		fcntl(skSocket, F_SETFL, O_NONBLOCK);
	}

	void SetSync()
	{
		ASYNC = 0;
	}

	BasicSocket()
	{
		SetSync();
		memset(&saAddress, 0, sizeof(sockaddr_in));
		memset(&saRemoteAddress, 0, sizeof(sockaddr_in));
		iPort = iLastSent = iLastRecv = iSent = iRecv = 0;
		Prot = TCP;
		Status = this->Created;

		iLastUsed = (unsigned int)time(0);
		skSocket = -1;
	}

	BasicSocket(BasicSocket &s)
	{
		saAddress=s.saAddress;
		saRemoteAddress=s.saRemoteAddress;
		iPort=s.iPort;
		iLastSent=s.iLastSent;
		iLastRecv=s.iLastRecv;
		iLastUsed = (unsigned int)time(0);
		this->iRecv=s.iRecv;
		iSent=s.iSent;
		Prot=s.Prot;
		skSocket=s.skSocket;
		Status=s.Status;
		s.skSocket=-1;
		s.Status=Closed;
	}

	~BasicSocket()
	{
		SetSync();
		Disconnect();
	}

	void Disconnect()
	{
		if(skSocket != -1)
		{
			close(skSocket);
			skSocket = -1;
		}
		Status = this->Closed;
	}

	int Bind(int iport)
	{
		if(Prot == TCP)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == -1)
			return false;

		int on = 1;
		setsockopt( skSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	
		if(iport == 0)
			iport = this->iPort;
		else
			this->iPort = iport;

		memset(&saAddress, 0, sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		saAddress.sin_port = htons(iPort);
		int res = ::bind(skSocket, (sockaddr*) &saAddress, sizeof(sockaddr));
		if( res < 0)
		{
			ErrorCheck();
			Disconnect();
			return false;
		}
		else
		{
			iLastUsed = (unsigned int)time(0);
			this->Status = this->Connected;
			//u_long iMode = 1;
			//ioctlsocket(skSocket, FIONBIO, &iMode);
			return true;
		}
	}

	int Listen()
	{
		int r = listen( skSocket, 64);
		if(r<0)
			ErrorCheck();
		return r;
	}

	int Accept(BasicSocket &skAcceptSocket)
	{
		socklen_t				iClientSize = sizeof(sockaddr_in);

		skAcceptSocket.skSocket = accept( skSocket, (struct sockaddr*)&skAcceptSocket.saAddress, &iClientSize );

		if(skAcceptSocket.skSocket == -1 )
		{
			ErrorCheck();
			return false;
		}
		else
		{
			iLastUsed = (unsigned int)time(0);
			skAcceptSocket.Prot = Prot;
			skAcceptSocket.iLastUsed = (unsigned int)time(0);
			//u_long iMode = 1;
			//ioctlsocket(skSocket, FIONBIO, &iMode);
			skAcceptSocket.Status = this->Connected;
			return true;
		}
	}

	void SetRemoteAddr(const char *szServerAddress, int iport)
	{
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saRemoteAddress.sin_addr.s_addr == INADDR_NONE)
			memcpy(&saRemoteAddress.sin_addr, gethostbyname(szServerAddress)->h_addr, 4);

		//Assign the port
		saRemoteAddress.sin_port = htons(iport);
	}

	int Connect(const char *szServerAddress, int iport)
	{
		//struct sockaddr_in	serv_addr;
		//LPHOSTENT			lpHost;
		int					err;
		if(iport==0)
			iport = this->iPort;
		else
			this->iPort = iport;

		Disconnect();
		//Open the socket
		if(Prot == TCP)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == -1)
			return false;

		//if(Prot == 0)
		//	Bind(iPort);

		memset(&saAddress,0,sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saAddress.sin_addr.s_addr == INADDR_NONE)
			memcpy(&saAddress.sin_addr, gethostbyname(szServerAddress)->h_addr, 4);

		//Assign the port
		saAddress.sin_port = htons(iPort);

		SetRemoteAddr(szServerAddress, iPort);
		//Establish the connection
		if(Prot == UDP)
		{
			iLastUsed = (unsigned int)time(0);
			//u_long iMode = 1;
			//ioctlsocket(skSocket, FIONBIO, &iMode);
			Status = this->Connected;
			return 1;
		}

		err = connect(skSocket, (struct sockaddr*)&saAddress, sizeof(sockaddr));
		if(err < 0)
		{
			ErrorCheck();
			Disconnect();
			return false;
		}
		iLastUsed = (unsigned int)time(0);
		//u_long iMode = 1;
		//ioctlsocket(skSocket, FIONBIO, &iMode);
		Status = this->Connected;
		return true;
	}


	int Send(const char *szBuffer, int iBufLen, int iFlags=0)
	{
		//cout << "Sending - \n"<<szBuffer<<"\n\n";
		if(Prot == TCP)//TCP
		{
			iLastSent = send(skSocket, szBuffer, iBufLen, ASYNC);
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastSent==-1)
				ErrorCheck("Send");

			return iLastSent;
		}
		else//UDP
		{
			iLastSent = sendto(skSocket, szBuffer, iBufLen, ASYNC, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastSent==-1)
				ErrorCheck("Send");

			return iLastSent;
		}
	}

	int Recv(char *szBuffer, int iBufLen, int iFlags=0)
	{
		if(Prot == TCP)//TCP
		{
			iLastRecv = recv(skSocket, szBuffer, iBufLen, ASYNC);
			if(iLastRecv>0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastRecv == 0)
				Status = Closed;
			else if(iLastRecv==-1)
				ErrorCheck("Recv");

			return iLastRecv;
		}
		else//UDP
		{
			//int len = sizeof(saRemoteAddress);
			iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, ASYNC, 0, 0);
			if(iLastRecv>0)
				iLastUsed = (unsigned int)time(0);
			else if(iLastRecv == 0)
				Status = Closed;
			else if(iLastRecv==-1)
				ErrorCheck("Recv");

			return iLastRecv;
		}
	}

	int SendTo(const char *szBuffer, int iBufLen, char *Address, int port, int iFlags=0)
	{
		iPort = port;
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(Address);
		saRemoteAddress.sin_port = htons(iPort);

		iLastSent = sendto(skSocket, szBuffer, iBufLen, ASYNC, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastSent==-1)
			ErrorCheck();
		return iLastSent;
	}

	int SendTo(const char *szBuffer, int iBufLen, sockaddr_in &address, int iFlags=0)
	{
		saRemoteAddress = address;

		iLastSent = sendto(skSocket, szBuffer, iBufLen, ASYNC, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastSent==-1)
			ErrorCheck();
		return iLastSent;
	}

	int RecvFrom(char *szBuffer, int iBufLen, int iFlags=0)
	{
		socklen_t a = sizeof(saRemoteAddress);
		iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, ASYNC, reinterpret_cast<sockaddr *>(&saRemoteAddress), &a);
		if(iLastRecv>0)
			iLastUsed = (unsigned int)time(0);
		else if(iLastRecv == 0)
				Status = Closed;
		else if(iLastRecv==-1)
			ErrorCheck();
		return iLastRecv;
	}

	void ErrorCheck(const char *function=NULL)
	{
		switch(errno)
		{
		/*case ENETDOWN:
		case ENOTCONN:
		case ENETRESET:
		case ENOTSOCK:
		case ESHUTDOWN:
		case ECONNABORTED:
		case ETIMEDOUT:
		case ECONNRESET:
			cout << "Closing connection("<<errno<<")!\n";
			perror(function);
			Status = Closed;
			break;*/
		case EAGAIN:
			break;
		default:
			cout << "Closing connection(def:"<<errno<<")!  ";
			perror(function);
			Status = Closed;
			//int a=0;
			//a=1/a;
			break;
		}
	}
};
#ifdef RAYHEADERSMAIN
int BasicSocket::SocketsInited = 0;
#endif

#endif

#define MAX_PAC_SIZE 4096//191

template<class Packet>
class PacketizedTCP : public BasicSocket
{
public:
	char buffer[MAX_PAC_SIZE+1];
	int buffpos;
	int front;

	PacketizedTCP()
	{
		Prot = TCP;
		buffer[0]='\0';
		buffpos=0;
		front=0;

		Packet* p = ((Packet*)(buffer+front));
		//p->Init(0, PAC_TYPES::DEFAULT);
	}

	/*PacketizedTCP(BasicSocket &sock) : BasicSocket(sock)
	{
		Prot = TCP;
		buffer[0]='\0';
		buffpos=0;
		front=0;

		Packet* p = ((Packet*)(buffer+front));
		p->SetLength(0);
	}*/

	void Clone(BasicSocket &sock)
	{
		BasicSocket *dst = (BasicSocket*)this;
		BasicSocket *src = &sock;

		memcpy((void*)dst, (void*)src, sizeof(BasicSocket));
	}

	int Send(const char *szBuffer, int iBufLen, int iFlags=0)
	{
		assert(iBufLen<=MAX_PAC_SIZE);
		int sent = 0;

		while(BasicSocket::Status == BasicSocket::Connected && iBufLen>0 )
		{
			BasicSocket::Send(szBuffer, iBufLen, iFlags);
			if(BasicSocket::iLastSent>0)
			{
				sent+=BasicSocket::iLastSent;
				szBuffer += BasicSocket::iLastSent;
				iBufLen -= BasicSocket::iLastSent;
			}
			else
				RaySleep(1000);
		}

		return sent;
	}

	int Send(const char *szBuffer)
	{
		int sent = 0;
		int iBufLen = ((Packet*)szBuffer)->GetLength();
		assert(iBufLen<=MAX_PAC_SIZE);
		int iFlags = 0;

		while(BasicSocket::Status == BasicSocket::Connected && iBufLen>0 )
		{
			BasicSocket::Send(szBuffer, iBufLen, iFlags);
			if(BasicSocket::iLastSent>0)
			{
				sent+=BasicSocket::iLastSent;
				szBuffer += BasicSocket::iLastSent;
				iBufLen -= BasicSocket::iLastSent;
			}

			else
				RaySleep(1000);
		}

		return sent;
	}

	char* Recv()
	{
		char *ret = NULL;

		if(front >= 0)
		{//do this at the front of the function so I don't overwrite new data
			if(buffpos-front>0)
				memmove(buffer, buffer+front, buffpos-front);
			buffpos -= front;
			front = 0;
		}

		BasicSocket::Recv(buffer+buffpos, MAX_PAC_SIZE-buffpos, 0);
		if(iLastRecv>0)
		{
			buffpos+=iLastRecv;
			buffer[buffpos]='\0';
			//fwrite(buffer+(buffpos-iLastRecv), 1, iLastRecv, stdout);
			//cout << "Received "<<iLastRecv<<" bytes\n";
			iLastUsed = (unsigned int)time(0);
		}

		int size = buffpos-front;
		if(size < sizeof(Packet))
			return ret;

		Packet* p = ((Packet*)(buffer+front));
		if(p->Checksum()==false)
		{
			cout << "Failed checksum test! Disconnecting!\n";
			Disconnect();
			return ret;
		}

		int len = p->GetLength();//ntohs(p->DataLen);
		if( len >= int(sizeof(Packet)) && len <= size )
		{
			ret = buffer+front;
			front += len;
		}
		return ret;
	}
};

template<class Packet>
class PacketObject
{
public:
	Packet pac;
	char *data;
	uint recved;

	PacketObject()
	{
		recved=(uint)time(0);
		data=NULL;
	}

	PacketObject(PacketObject &b)
	{
		recved=b.recved;
		pac=b.pac;
		data=NULL;
		if(b.data)
		{
			data=new char[pac.GetLength()-sizeof(Packet)];
			memcpy(data, b.data, pac.GetLength()-sizeof(Packet));
		}
	}

	PacketObject(char *buff)
	{
		recved=(uint)time(0);
		pac = ((Packet*)buff);
		int len = pac.GetLength()-sizeof(Packet);
		if(len>0)
		{
			data = new char[len];
			memcpy(data, buff+sizeof(Packet), len);
		}
	}

	void Init(char *buff)
	{
		recved=(uint)time(0);
		pac = *((Packet*)buff);
		int len = pac.GetLength()-sizeof(Packet);
		if(len>0)
		{
			data = new char[len];
			memcpy(data, buff+sizeof(Packet), len);
		}
	}

	void MakePart(char *buff, int recvd)
	{
		recved=(uint)time(0);
		assert(recved>=sizeof(Packet));
		pac = *((Packet*)buff);
		int len = pac.GetLength()-sizeof(Packet);
		if(len>0)
		{
			data = new char[len];
			if(recvd>sizeof(Packet))
				memcpy(data, buff+sizeof(Packet), recvd-sizeof(Packet));
			else
				data[0]='\0';
		}
	}

	~PacketObject()
	{
		delete[] data;
	}
};

template<class Packet>
class RequestResponseTCP : public BasicSocket
{
public:
	char buffer[MAX_PAC_SIZE];
	int buffpos;
	int front;

	LinkedList2< PacketObject<Packet>,int,NewAlloc> pacs;
	PacketObject<Packet> *CurrentPacket;
	typedef PacketObject<Packet> PacObj;

	RequestResponseTCP()
	{
		CurrentPacket=NULL;
		Prot = TCP;
		buffer[0]='\0';
		buffpos=0;
		front=0;

		Packet* p = ((Packet*)(buffer+front));
		//p->Init(0, PAC_TYPES::DEFAULT);
	}

	void Clone(BasicSocket &sock)
	{
		iLastUsed = (unsigned int)time(0);
		BasicSocket *dst = (BasicSocket*)this;
		BasicSocket *src = &sock;

		memcpy((void*)dst, (void*)src, sizeof(BasicSocket));
	}

	int Send(const char *szBuffer, int iBufLen, int iFlags=0, uint maxtries=UINT_MAX)
	{
		//assert(iBufLen<=MAX_PAC_SIZE);
		int sent = 0;
		uint tries = 0;

		while(BasicSocket::Status == BasicSocket::Connected && iBufLen>0 )
		{
			BasicSocket::Send(szBuffer, iBufLen, iFlags);
			if(BasicSocket::iLastSent>0)
			{
				tries=0;
				sent+=BasicSocket::iLastSent;
				szBuffer += BasicSocket::iLastSent;
				iBufLen -= BasicSocket::iLastSent;
			}
			else if(tries>maxtries)
			{
				/*cout << "Failed on send for type "<<(int)((Packet*)szBuffer)->GetType()<<"!\n";
				BasicSocket::Disconnect();
				//int a=0;
				//a=1/0;
				return -1;*/
				return sent;
			}
			else
			{
				tries++;
				RaySleep(1000);
			}
		}

		return sent;
	}

	int Send(const char *szBuffer)//, uint maxtries=UINT_MAX)
	{
		int sent = 0;
		int iBufLen = ((Packet*)szBuffer)->GetLength();
		//assert(iBufLen<=MAX_PAC_SIZE);
		int iFlags = 0;
		uint tries = 0;

		while(Status == BasicSocket::Connected && iBufLen>0 )
		{
			//Recv();
			BasicSocket::Send(szBuffer, iBufLen, iFlags);
			if(iLastSent>0)
			{
				tries=0;
				sent+=iLastSent;
				szBuffer += iLastSent;
				iBufLen -= iLastSent;
			}
			else if(tries>UINT_MAX)
			{
				/*cout << "Failed on send for type "<<(int)((Packet*)szBuffer)->GetType()<<"!\n";
				BasicSocket::Disconnect();
				//int a=0;
				//a=1/0;
				return -1;*/
				return sent;
			}
			else
			{
				tries++;
				RaySleep(1000);
			}
		}

		return sent;
	}

	void DeleteOldPacs()
	{
		for(PacketObject<Packet> *t = pacs.GetFirst();t;t=pacs.GetNext(t))
		{
			while( (uint)time(0)-t->recved > 60 )
			{
				pacs.Delete(t);
				t = pacs.GetFirst();
			}
		}
	}

	char* RecvSmall()
	{
		char *ret = NULL;

		if(front >= 0)
		{//do this at the front of the function so I don't overwrite new data
			if(buffpos-front>0)
				memmove(buffer, buffer+front, buffpos-front);
			buffpos -= front;
			front = 0;
		}

		BasicSocket::Recv(buffer+buffpos, MAX_PAC_SIZE-buffpos, 0);
		if(iLastRecv>0)
		{
			buffpos+=iLastRecv;
			//buffer[buffpos-1]='\0';
			//fwrite(buffer+(buffpos-iLastRecv), 1, iLastRecv, stdout);
			//cout << "Received "<<iLastRecv<<" bytes\n";
			iLastUsed = (unsigned int)time(0);
		}

		int size = buffpos-front;
		if(size < sizeof(Packet))
			return ret;

		Packet* p = ((Packet*)(buffer+front));
		if(p->Checksum()==false)
		{
			cout << "Failed checksum test! Disconnecting!\n";
			Disconnect();
			front=buffpos=0;
			return ret;
		}

		int len = p->GetLength();//ntohs(p->DataLen);
		if( len >= int(sizeof(Packet)) && len <= size )
		{
			if(buffer[len-1]!='\0')
			{
				//cout << buffer << "\n";
				cout << "if(buffer[len-1]!=\'\\0\')\n";
				Disconnect();
				front=buffpos=0;
				return ret;
			}
			//buffer[buffpos-1]='\0';
			ret = buffer+front;
			front += len;
		}
		return ret;
	}

	PacketObject<Packet> *Recv()
	{
		if(CurrentPacket==NULL)
		{
			if(char *r = RecvSmall())
			{
				PacObj *p = pacs.template Create<PacObj>();
				p->Init(r);
				return pacs.GetFirst();
				return p;
			}
			else if(buffpos>=sizeof(Packet) && ((Packet*)buffer)->GetLength()>MAX_PAC_SIZE)
			{
				CurrentPacket = new PacketObject<Packet>();
				CurrentPacket->MakePart(buffer, buffpos);
				return pacs.GetFirst();
				return NULL;
			}
		}
		else
		{
			BasicSocket::Recv(CurrentPacket->data+buffpos-sizeof(Packet), CurrentPacket->pac.GetLength()-buffpos, 0);
			if(iLastRecv>0)
			{
				buffpos+=iLastRecv;
				//CurrentPacket->data[buffpos-1]='\0';
				iLastUsed = (unsigned int)time(0);
				if(CurrentPacket->pac.GetLength()==buffpos)
				{
					if(CurrentPacket->data[CurrentPacket->pac.GetLength()-sizeof(Packet)-1]!='\0')
					{
						cout << "if(CurrentPacket->data[CurrentPacket->pac.GetLength()-sizeof(Packet)-1]!='\0')\n";
						Disconnect();
						front=buffpos=0;
						delete CurrentPacket;
						CurrentPacket = NULL;
						DeleteOldPacs();
						return pacs.GetFirst();
						return NULL;
					}
					//CurrentPacket->data[buffpos-1-sizeof(Packet)]='\0';
					PacketObject<Packet> *p = pacs.template Create<PacObj>();
					p->pac = CurrentPacket->pac;
					p->data = CurrentPacket->data;
					CurrentPacket->data = NULL;
					delete CurrentPacket;
					CurrentPacket = NULL;
					front=0;
					buffpos=0;
					DeleteOldPacs();
					return pacs.GetFirst();
					return p;
				}
			}
		}
		return pacs.GetFirst();
		return NULL;
	}

	/*PacketObject<Packet> *GetResponse(char *buff, int tries=5000, CriticalSection *cs=NULL)
	{
		Packet &pac = *((Packet*)buff);

		if(cs)
			cs->lock();

		//cout << "GetResponse...\n";
		for(;tries && Status==BasicSocket::Connected;tries--)
		{
			//cout << "Trying to receive, "<<tries<<" tries left...\n";
			for(PacketObject<Packet> *p = pacs.GetFirst();p;p=pacs.GetNext(p))
			{
				if(pac.IsResponse(p->pac))
				{
					if(cs)
						cs->unlock();
					return p;
				}
			}

			PacketObject<Packet> *r = Recv();
			if(cs)

				cs->unlock();
			if(r && pac.IsResponse(r->pac))
				return r;
			else
				RaySleep(1000);
			if(cs)
				cs->lock();
		}

		BasicSocket::Disconnect();
		cs->unlock();
		cout << "Failed on GetResponse for "<<(int)pac.GetType()<<"!\n";
		//int a=0;
		//a=1/a;
		return NULL;
	}*/

	PacketObject<Packet> *GetResponse(char *buff, int tries=5000)
	{
		Packet &pac = *((Packet*)buff);

		for(PacketObject<Packet> *p = pacs.GetFirst();p;p=pacs.GetNext(p))
			if(pac.IsResponse(p->pac))
				return p;

		for(;tries && Status==BasicSocket::Connected;tries--)
		{
			//cout << "Trying to receive, "<<tries<<" tries left...\n";
			PacketObject<Packet> *r = Recv();
			if(r && pac.IsResponse(r->pac))
				return r;
			else
				RaySleep(1000);
		}

		//BasicSocket::Disconnect();
		cout << "Failed on GetResponse for "<<(int)pac.GetType()<<"!\n";
		//int a=0;
		//a=1/a;
		return NULL;
	}

	PacketObject<Packet> *GetResponse(char *buff, CriticalSection &cs, int tries=5000)
	{
		Packet &pac = *((Packet*)buff);

		tries /= 100;
		for(int locks=tries;locks>=0;locks--)
		{
			cs.cslock();
			if(Status!=Connected)
			{
				cs.csunlock();
				break;
			}
			for(PacketObject<Packet> *p = pacs.GetFirst();p;p=pacs.GetNext(p))
			{
				if(p->recved<(uint)time(0)-600)
				{
					cout << "Deleting old packet of type "<<(uint)p->pac.GetType()<<"\n";
					pacs.Delete(p);
					p=pacs.GetFirst();
					if(!p)
						break;
				}
				if(pac.IsResponse(p->pac))
				{
					cs.csunlock();
					return p;
				}
			}

			for(tries=100;tries>=0 && Status==BasicSocket::Connected;tries--)
			{
				//cout << "Trying to receive, "<<tries<<" tries left...\n";
				PacketObject<Packet> *r = Recv();
				if(r && pac.IsResponse(r->pac))
				{
					cs.csunlock();
					return r;
				}
				else
					RaySleep(1000);
			}
			cs.csunlock();
			RaySleep(1000);
		}

		//BasicSocket::Disconnect();
		cout << "Failed on GetResponse for "<<(int)pac.GetType()<<"!\n";
		//int a=0;
		//a=1/a;
		return NULL;
	}

	PacketObject<Packet> *Request(char *buff, int tries, CriticalSection *cs)
	{
		if(cs)
			cs->cslock();
		Send(buff);
		if(cs)
		{
			cs->csunlock();
			return GetResponse(buff, *cs, tries);
		}
		else
			return GetResponse(buff, tries);
	}
};
