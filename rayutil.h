#pragma once
#include <assert.h>
#include <binary.h>

#ifndef BDEBUG
#ifdef _DEBUG
#define BDEBUG 1
#else
#define BDEBUG 0
#endif
#endif

__inline int log2(int a)
{
	int i=31;
	for(; i>1 && (a & (1<<i)) == 0; i--);
	return i;
}

#ifdef NO_STATIC_ASSERT
#define STATIC_ASSERT( B ) (B)
#else
template <bool> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true>{ enum { value = 1 }; };
//template <> struct STATIC_ASSERTION_FAILURE<false>{ enum { value = 1 }; };
template<int x> struct static_assert_test{};

#define STATIC_ASSERT( B ) \
   typedef ::static_assert_test<\
      sizeof(::STATIC_ASSERTION_FAILURE< bool ( B ) >)>\
         boost_static_assert_typedef_
#endif

#ifdef  NDEBUG
#define debugmess(text)     ((void)0)
#else
#define debugmess(text)     (text)
#endif

#define ALWAYS 0
#define FATAL 2
#define ERR 4
#define WARN 6
#define INFO 8
#define DEBUG 10
#define TRACE 12
#define CALLSRETURNS 13

#if !defined(DEBUGLEVEL) || !defined(DEBUGFILELEVEL)
//#error DEBUGLEVEL and DEBUGFILELEVEL must be defined, DEBUGLOG to enable, 0,1=ALWAYS, 2,3=FATAL, 4,5=ERROR, 6,7=WARNING, 8,9=INFO, 10,11=DEBUG, 12+=TRACE, 13=Calls and Returns
#else
#define DEBUGLOG
const int DEBUGANYLEVEL=max(DEBUGLEVEL,DEBUGFILELEVEL);
#endif

#ifdef DEBUGLOG
class CALLERBASE
{
public:
	const char *file;
	const char *func;
	int line;

	CALLERBASE(const char *File, const char * Func, int Line)
	{
		file=File;
		line=Line;
		func=Func;
	}

	~CALLERBASE()
	{
	}
};

extern vector< CALLERBASE > stacktrace;
#ifndef DEBUGMULTITHREAD
class NullCriticalSection
{
public:
#ifndef cslock
	void cslock() {}
#endif
	void _lock(const char *file, unsigned int line) {}
	void csunlock() {}
};
extern NullCriticalSection stacktracecs;
#else
extern CriticalSection stacktracecs;
#endif
#ifdef RAYHEADERSMAIN
vector< CALLERBASE > stacktrace;
#ifndef DEBUGMULTITHREAD
NullCriticalSection stacktracecs;
#else
CriticalSection stacktracecs;
#endif
#endif

//__func__ may be an alternative to __FUNCTION__, __PRETTY_FUNCTION__ looks good too
#ifdef WIN32
#define __PRETTY_FUNCTION__ __FUNCTION__"()"
#endif

#ifdef _DEBUG
#define DEBUGARGDEF CALLER caller
#define DEBUGARG CALLER(__FILE__,__PRETTY_FUNCTION__,__LINE__)
#define DEBUGARGSDEF ,CALLER caller
#define DEBUGARGS ,CALLER(__FILE__,__PRETTY_FUNCTION__,__LINE__)
#define CALLERFILE caller.file
#define CALLERLINE caller.line
#else
#define DEBUGARGDEF
#define DEBUGARG
#define DEBUGARGSDEF
#define DEBUGARGS
#define CALLERFILE ""
#define CALLERLINE 0
#endif

extern std::ofstream debugfile;
#ifdef RAYHEADERSMAIN
#if DEBUGFILELEVEL >= 0
/*#ifndef DEBUGFILENAME
#define DEBUGFILENAME "debugout.txt"
#endif*/
std::ofstream debugfile( DEBUGFILENAME, std::ofstream::app);
#else
std::ofstream debugfile;
#endif
#endif

#ifdef RAYHEADERSMAIN
string TimestampString()
{
	time_t rawtimestamp = time(0);
	char buffer[128];
	tm * timeinfo;
	timeinfo=localtime(&rawtimestamp);
	strftime(buffer,128,"%c",timeinfo);
	return string(buffer);
}
#else
string TimestampString();
#endif

//#define DEBUGOUT( level, A ) do { if(level<=DEBUGLEVEL) cerr << #A << " == \""<<(A)<<"\", "<< __FILE__ << ":" << __LINE__ << "\n"; } while(0)
#define DEBUGMESS2( level, MAXLEVEL, A, A2, B, ofstr)\
do { \
	if(level<=MAXLEVEL) {\
		const char * slevel="ALWAYS\0FATAL\0\0ERR\0\0\0\0WARN\0\0\0INFO\0\0\0DEBUG\0\0TRACE\0\0";\
		const int tlevel=min((int)level/2,(int)5);\
		slevel+=tlevel*7;\
		std::stringstream tos;\
		const char *tabs="    ""    ""    ""    ""    ""    ""    ""    ";\
		stacktracecs.cslock();\
		tos << tabs+(32-min(32,(int)stacktrace.size()*2));\
		stacktracecs.csunlock();\
		tos << "("<<slevel<<"="<<level<<") "<<TimestampString()<<": \"" << A << "\" == \""<<(A2)<<"\", "<<B<<", \t"<< __FILE__ << " :\t" << __PRETTY_FUNCTION__ << "\t:" << __LINE__ << "\n"; \
		ofstr << tos.str();\
		ofstr.flush();\
	}\
} while(0)

#define DEBUGMESSPLAIN2( level, MAXLEVEL, A, B, ofstr)\
do { \
	if(level<=MAXLEVEL) {\
		const char * slevel="ALWAYS\0FATAL\0\0ERR\0\0\0\0WARN\0\0\0INFO\0\0\0DEBUG\0\0TRACE\0\0";\
		const int tlevel=min((int)level/2,(int)6);\
		slevel+=tlevel*7;\
		std::stringstream tos;\
		const char *tabs="    ""    ""    ""    ""    ""    ""    ""    ";\
		stacktracecs.cslock();\
		tos << tabs+(32-min(32,(int)stacktrace.size()*2));\
		stacktracecs.csunlock();\
		tos << A << "("<<slevel<<"="<<level<<") "<<TimestampString()<<": "<< B << "\n"; \
		ofstr << tos.str();\
		ofstr.flush();\
	}\
} while(0)

#define DEBUGMESSBLANK2( level, MAXLEVEL, A, ofstr)\
do { \
	if(level<=MAXLEVEL) {\
		std::stringstream tos;\
		const char *tabs="    ""    ""    ""    ""    ""    ""    ""    ";\
		stacktracecs.cslock();\
		tos << tabs+(32-min(32,(int)stacktrace.size()*2));\
		stacktracecs.csunlock();\
		tos << A << "\n";\
		ofstr << tos.str();\
		ofstr.flush();\
	}\
} while(0)

#define DEBUGMESS( level, A, A2, B)\
do { \
	DEBUGMESS2(level,DEBUGLEVEL, A, A2, B, cerr);\
	DEBUGMESS2(level,DEBUGFILELEVEL, A, A2, B, debugfile);\
} while(0)

#define DEBUGMESSPLAIN( level, A, B)\
do { \
	DEBUGMESSPLAIN2(level,DEBUGLEVEL, A, B, cerr);\
	DEBUGMESSPLAIN2(level,DEBUGFILELEVEL, A, B, debugfile);\
} while(0)

#define DEBUGMESSBLANK( level, A)\
do { \
	DEBUGMESSBLANK2(level,DEBUGLEVEL, A, cerr);\
	DEBUGMESSBLANK2(level,DEBUGFILELEVEL, A, debugfile);\
} while(0)

#define DEBUGOUTSTACK( level, A, B ) \
do { \
	if(level<=DEBUGANYLEVEL) {\
		DEBUGMESS(level,#A,A,B);\
		if(level<DEBUGANYLEVEL) {\
			int levelcount=level+1;\
			DEBUGMESSPLAIN(levelcount,"\t","stacktrace ==");\
			stacktracecs.cslock();\
			for(size_t c=stacktrace.size()-1;c<stacktrace.size() && /*c!=stacktrace.size()-5 &&*/ levelcount<=DEBUGANYLEVEL;c--,levelcount++) {\
				DEBUGMESSPLAIN(levelcount,"\t\t", stacktrace[c].func <<":"<< stacktrace[c].line);\
			}\
			stacktracecs.csunlock();\
			DEBUGMESSBLANK(level+1,"");\
		}\
	}\
} while(0)

#define DEBUGOUT( level, A, B ) \
do { \
	if(level<=DEBUGANYLEVEL) {\
		DEBUGMESS(level,#A,A,B);\
		DEBUGMESSBLANK(level,"");\
	}\
} while(0)
		
#define DEBUGOUTMARKER( level ) \
do { \
	CALLER c(__FILE__,__PRETTY_FUNCTION__,__LINE__); \
	DEBUGOUTSTACK(level, c.func, "line "<<c.line); \
} while(0)

class CALLER : public CALLERBASE
{
private:
	bool pop;
	CALLER(CALLER &old) : CALLERBASE(old)
	{
		//throw std::exception("CALLER(&old)");
	}
public:
	CALLER(CALLER &&old) : CALLERBASE(old)
	{
		//throw std::exception("CALLER(&old)");
		//DEBUGMESSPLAIN(1,"","Copying CALLER "<<func<<":"<<line);
		old.pop=false;
		pop=true;
	}

	CALLER(const char *File, const char * Func, int Line) : CALLERBASE(File,Func,Line)
	{
		//cout << "CALLER("<<file<<":"<<func<<":"<<line<<")\n";
#ifndef NOSTACK
		pop=true;
		stacktracecs.cslock();
		DEBUGMESSPLAIN(CALLSRETURNS,"","Calling from "<<func<<":"<<line);
		stacktrace.push_back( *this );
		stacktracecs.csunlock();
#endif
	}

	~CALLER()
	{
		//cout << "~CALLER("<<fle<<":"<<func<<":"<<line<<")\n";
#ifndef NOSTACK
		if(pop) {
			DEBUGMESSPLAIN(CALLSRETURNS,"","Returning to "<<func<<":"<<line);
			stacktracecs.cslock();
			if(stacktrace.size()) stacktrace.pop_back();
			else cerr << "\nwarning: empty stack?\n";
			stacktracecs.csunlock();
		}
#endif
	}
};

#else
#define DEBUGARGDEF
#define DEBUGARG
#define DEBUGARGSDEF
#define DEBUGARGS
#define CALLERFILE ""
#define CALLERLINE 0
#define DEBUGOUT( level, A, B ) ((void)0)
#define DEBUGOUTSTACK( level, A, B ) ((void)0)
#define DEBUGOUTMARKER( level ) ((void)0)
#endif


typedef unsigned int uint;

template<class T>
unsigned int QSPartition(T *p, unsigned int len, unsigned int pivot)
{
	len--;
	T t;
	T pivotValue = p[pivot];
	t = p[len];
	p[len] = p[pivot];
	p[pivot] = t;
	unsigned int storeIndex = 0;

	for(unsigned int i=0;i<len;i++)
	{
		if(p[i] <= pivotValue)
		{
			t = p[i];
			p[i] = p[storeIndex];
			p[storeIndex] = t;
			storeIndex++;
		}
	}

	t = p[len];
	p[len] = p[storeIndex];
	p[storeIndex] = t;
	return storeIndex;
}

template<class T>
void QuickSort(T *p, unsigned int len)
{
	if(len<=1)
		return;
	unsigned int pivot = len/2;
	pivot = QSPartition(p, len, pivot);

	QuickSort(p, pivot);
	QuickSort(p+pivot, len-pivot);

}

#ifndef RAYHEADERSMAIN
template<>
unsigned int QSPartition<string>(string *p, unsigned int len, unsigned int pivot);
#else
template<>
unsigned int QSPartition<string>(string *p, unsigned int len, unsigned int pivot)
{
	len--;
	string t;
	string pivotValue = p[pivot];
	t = p[len];
	p[len] = p[pivot];
	p[pivot] = t;
	unsigned int storeIndex = 0;

	for(unsigned int i=0;i<len;i++)
	{
		//if(p[i] <= pivotValue)
		if( strcmp(p[i].c_str(), pivotValue.c_str()) <= 0 )
		{
			t = p[i];
			p[i] = p[storeIndex];
			p[storeIndex] = t;
			storeIndex++;
		}
	}

	t = p[len];
	p[len] = p[storeIndex];
	p[storeIndex] = t;
	return storeIndex;
}
#endif
