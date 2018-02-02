#ifdef WIN32
#include <winsock2.h>
#endif
#ifdef PHONE
#include <ws2tcpip.h>
#endif

#ifdef WIN32

class BasicSocket : public ContObj<BasicSocket>
{
public:
	static int SocketsInited;

	int iPort;
	int Prot;
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
	SStatuses Status;

	/*void Disconnect();
	int Bind(int iport);
	int Listen();
	int Connect(char* szServerAddress, int iport);
	int Accept(BasicSocket &skAcceptSocket);
	int Send(char *szBuffer, int iBufLen, int iFlags);
	int Recv(char *szBuffer, int iBufLen, int iFlags);*/

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
		iPort = Prot = iLastSent = iLastRecv = iSent = iRecv = 0;
		Status = this->Created;

		iLastUsed = (unsigned int)time(0);
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

	int Bind(int iport)
	{
		if(Prot == 1)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == INVALID_SOCKET)
			return false;

		if(iport == 0)
			iport = this->iPort;
		else
			this->iPort = iport;

		memset(&saAddress, 0, sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		saAddress.sin_port = htons(iPort);
		if( bind(skSocket, (sockaddr*) &saAddress, sizeof(sockaddr)) == SOCKET_ERROR)
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
			iLastUsed = (unsigned int)time(0);
			skAcceptSocket.iLastUsed = (unsigned int)time(0);
			u_long iMode = 1;
			ioctlsocket(skSocket, FIONBIO, &iMode);
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

	int Connect(char *szServerAddress, int iport)
	{
		//struct sockaddr_in	serv_addr;
		//LPHOSTENT			lpHost;
		int					err;
		if(iport==0)
			iport = this->iPort;
		else
			this->iPort = iport;

		/*char TName[128];
		if(Prot == 1)
		strcpy(TName, "TCP ");
		else
		strcpy(TName, "UDP ");

		strcpy(&TName[strlen(TName)], szServerAddress);
		sprintf( &TName[strlen(TName)], ":%d", iport);
		name = new char[strlen(TName)+1];
		strcpy(name, TName);*/

		Disconnect();
		//Open the socket
		if(Prot == 1)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == INVALID_SOCKET)
			return false;

		//if(Prot == 0)
		//	Bind(iPort);

		memset(&saAddress,0,sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saAddress.sin_addr.s_addr == INADDR_NONE)
			memcpy(&saAddress.sin_addr.S_un, gethostbyname(szServerAddress)->h_addr, 4);

		//Assign the port
		saAddress.sin_port = htons(iPort);

		SetRemoteAddr(szServerAddress, iPort);
		//Establish the connection
		if(Prot == 0)
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
		u_long iMode = 1;
		ioctlsocket(skSocket, FIONBIO, &iMode);
		Status = this->Connected;
		return true;
	}


	int Send(char *szBuffer, int iBufLen, int iFlags=0)
	{
		//cout << "Sending - \n"<<szBuffer<<"\n\n";
		if(Prot == 1)//TCP
		{
			iLastSent = send(skSocket, szBuffer, iBufLen, iFlags);
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			return iLastSent;
		}
		else//UDP
		{
			iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			return iLastSent;
		}
	}

	int Recv(char *szBuffer, int iBufLen, int iFlags=0)
	{
		if(Prot == 1)//TCP
		{
			iLastRecv = recv(skSocket, szBuffer, iBufLen, iFlags);
			return iLastRecv;
		}
		else//UDP
		{
			//int len = sizeof(saRemoteAddress);
			iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, iFlags, 0, 0);
			return iLastRecv;
		}
	}

	int SendTo(char *szBuffer, int iBufLen, char *Address, int port, int iFlags=0)
	{
		iPort = port;
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(Address);
		saRemoteAddress.sin_port = htons(iPort);

		iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		return iLastSent;
	}

	int SendTo(char *szBuffer, int iBufLen, sockaddr_in &address, int iFlags=0)
	{
		saRemoteAddress = address;

		iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		return iLastSent;
	}

	int RecvFrom(char *szBuffer, int iBufLen, int iFlags=0)
	{
		int a = sizeof(saRemoteAddress);
		iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), &a);
		return iLastRecv;
	}
};

int BasicSocket::SocketsInited = 0;

#endif

#ifdef LINUX

class BasicSocket : public ContObj<BasicSocket>
{
public:
	static int SocketsInited;

	int iPort;
	int Prot;
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
	SStatuses Status;

	/*void Disconnect();
	int Bind(int iport);
	int Listen();
	int Connect(char* szServerAddress, int iport);
	int Accept(BasicSocket &skAcceptSocket);
	int Send(char *szBuffer, int iBufLen, int iFlags);
	int Recv(char *szBuffer, int iBufLen, int iFlags);*/

	BasicSocket()
	{
		memset(&saAddress, 0, sizeof(sockaddr_in));
		memset(&saRemoteAddress, 0, sizeof(sockaddr_in));
		iPort = Prot = iLastSent = iLastRecv = iSent = iRecv = 0;
		Status = this->Created;

		iLastUsed = (unsigned int)time(0);
		skSocket = 0;
	}

	~BasicSocket()
	{
		Disconnect();
	}

	void Disconnect()
	{
		if(skSocket != 0)
		{
			close(skSocket);
			skSocket = 0;
		}
		Status = this->Closed;
	}

	int Bind(int iport)
	{
		if(Prot == 1)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == 0)
			return false;

		if(iport == 0)
			iport = this->iPort;
		else
			this->iPort = iport;

		memset(&saAddress, 0, sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		saAddress.sin_port = htons(iPort);
		if( bind(skSocket, (sockaddr*) &saAddress, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			//ErrorCheck();
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
		return listen( skSocket, 64);
	}

	int Accept(BasicSocket &skAcceptSocket)
	{
		int				iClientSize = sizeof(sockaddr_in);

		skAcceptSocket.skSocket = accept( skSocket, (struct sockaddr*)&skAcceptSocket.saAddress, &iClientSize );

		if(skAcceptSocket.skSocket == 0 )
		{
			return false;
		}
		else
		{
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

	int Connect(char *szServerAddress, int iport)
	{
		//struct sockaddr_in	serv_addr;
		//LPHOSTENT			lpHost;
		int					err;
		if(iport==0)
			iport = this->iPort;
		else
			this->iPort = iport;

		/*char TName[128];
		if(Prot == 1)
		strcpy(TName, "TCP ");
		else
		strcpy(TName, "UDP ");

		strcpy(&TName[strlen(TName)], szServerAddress);
		sprintf( &TName[strlen(TName)], ":%d", iport);
		name = new char[strlen(TName)+1];
		strcpy(name, TName);*/

		Disconnect();
		//Open the socket
		if(Prot == 1)
			skSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			skSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(skSocket == 0)
			return false;

		//if(Prot == 0)
		//	Bind(iPort);

		memset(&saAddress,0,sizeof(sockaddr_in));
		saAddress.sin_family = AF_INET;
		saAddress.sin_addr.s_addr = inet_addr(szServerAddress);

		if(saAddress.sin_addr.s_addr == INADDR_NONE)
			memcpy(&saAddress.sin_addr.S_un, gethostbyname(szServerAddress)->h_addr, 4);

		//Assign the port
		saAddress.sin_port = htons(iPort);

		SetRemoteAddr(szServerAddress, iPort);
		//Establish the connection
		if(Prot == 0)
		{
			iLastUsed = (unsigned int)time(0);
			//u_long iMode = 1;
			//ioctlsocket(skSocket, FIONBIO, &iMode);
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


	int Send(char *szBuffer, int iBufLen, int iFlags=0)
	{
		//cout << "Sending - \n"<<szBuffer<<"\n\n";
		if(Prot == 1)//TCP
		{
			iLastSent = send(skSocket, szBuffer, iBufLen, MSG_DONTWAIT);
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			return iLastSent;
		}
		else//UDP
		{
			iLastSent = sendto(skSocket, szBuffer, iBufLen, MSG_DONTWAIT, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
			if(iLastSent>0)
				iLastUsed = (unsigned int)time(0);
			return iLastSent;
		}
	}

	int Recv(char *szBuffer, int iBufLen, int iFlags=0)
	{
		if(Prot == 1)//TCP
		{
			iLastRecv = recv(skSocket, szBuffer, iBufLen, MSG_DONTWAIT);
			return iLastRecv;
		}
		else//UDP
		{
			//int len = sizeof(saRemoteAddress);
			iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, MSG_DONTWAIT, 0, 0);
			return iLastRecv;
		}
	}

	int SendTo(char *szBuffer, int iBufLen, char *Address, int port, int iFlags=0)
	{
		iPort = port;
		memset(&saRemoteAddress,0,sizeof(sockaddr_in));
		saRemoteAddress.sin_family = AF_INET;
		saRemoteAddress.sin_addr.s_addr = inet_addr(Address);
		saRemoteAddress.sin_port = htons(iPort);

		iLastSent = sendto(skSocket, szBuffer, iBufLen, MSG_DONTWAIT, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		return iLastSent;
	}

	int SendTo(char *szBuffer, int iBufLen, sockaddr_in &address, int iFlags=0)
	{
		saRemoteAddress = address;

		iLastSent = sendto(skSocket, szBuffer, iBufLen, iFlags, reinterpret_cast<sockaddr *>(&saRemoteAddress), sizeof(sockaddr_in));
		if(iLastSent>0)
			iLastUsed = (unsigned int)time(0);
		return iLastSent;
	}

	int RecvFrom(char *szBuffer, int iBufLen, int iFlags=0)
	{
		int a = sizeof(saRemoteAddress);
		iLastRecv = recvfrom(skSocket, szBuffer, iBufLen, MSG_DONTWAIT, reinterpret_cast<sockaddr *>(&saRemoteAddress), &a);
		return iLastRecv;
	}
};

int BasicSocket::SocketsInited = 0;

#endif
