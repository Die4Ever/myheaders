class DBSocket;
class Results;

class cQuery
{
public:
	DBSocket *sock;
	string buff;
	int queryid;
	Results *res;

	void Execute();
	void GetResults();

	cQuery()
	{
		res=0;
		queryid=-1;
		sock=0;
	}
};

class Cell
{
public:
	int start;
	int length;
};

class Results : public ContObj<Results>
{
public:
	int qid;
	int length;
	int rows;
	int columns;
	string buff;
	Cell *cells;
	int done;

	Results()
	{
		cells = 0;
		length = 0;
		rows = 0;
		columns = 0;
		qid = -1;
		done = 0;
	}

	string GetCell(int row, int col)
	{
		return buff.substr(cells[row*columns +col].start, cells[row*columns +col].length);
	}

	int SortComp(Results *objB)
	{
		if(qid > objB->qid)
			return 1;
		else if(qid < objB->qid)
			return -1;
		return 0;
	}

	int SortComp(char *objB)
	{
		if(qid > ((int)objB))
			return 1;
		else if(qid < ((int)objB))
			return -1;
		return 0;
	}
};

class DBSocket : public BasicSocket
{
public:
	int recving;
	int currqueryid;
	string buff;

	Results *tres;
	LinkedList<Results> results;
	CriticalSection cs;

	DBSocket()
	{
		Prot = 1;
		tres = 0;
		recving = 0;
		currqueryid = 0;
	}

	int RecvResults()
	{
		cs.lock();
		
		cs.unlock();
		return 1;
	}

	cQuery GetQuery()
	{
		cQuery t;
		t.queryid = currqueryid++;
		t.sock = this;
		return t;
	}
};

void cQuery::Execute()
{
	string s;
	s.resize(128);

	_itoa_s(strlen(buff.c_str()),(char*)s.c_str(), s.size(),10);

	unsigned int len = strlen( (char*)s.c_str() );
	s[len++] = '\n';
	s[len] = '\0';

	_itoa_s(queryid,(char*)&s.c_str()[len], s.size()-len,10);

	len = strlen( (char*)s.c_str() );
	s[len++] = '\n';
	s[len] = '\0';

	sock->Send((char*)s.c_str(), len);

	len = strlen( (char*)buff.c_str() );
	if( buff[len-1] != ';' )
	{
		if( buff.size() < len+1)
			buff.resize(len+1);
		buff[len] = ';';
		len++;
	}
	unsigned int i=0;
	while(i<len)
	{
		sock->Send((char*)&buff[i], len-i);
		if(sock->iLastSent>0)
			i+=sock->iLastSent;
		if(i<len)
			RaySleep(1000);
	}
}

void cQuery::GetResults()
{
	res=0;
	while(res==0)
	{
		if(sock->RecvResults()==0)
			return;
		res = sock->results.Get( (char*)queryid );
		RaySleep(1);
	}
	while(res->done == 0)
	{
		if(sock->RecvResults()==0)
			return;
		RaySleep(1000);
	}
	res->Del();
}
