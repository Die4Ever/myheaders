/*
TODO:
-update the hash function to the shorter one I use
-do something about the ConnectToMysql() function taking login info
-use splitbys for parsing query vars?
-maybe keep the raw post data somewhere besides just in Variables?
*/
//#define _CRT_SECURE_NO_WARNINGS

extern const char* USER_AGENT;
extern bool cache_server;

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <time.h>
#include <ctime>
#include <array>
//using namespace std;
using std::string;
using std::wstring;
using std::vector;
using std::stringstream;
using std::cerr;
using std::cout;
using std::array;
using std::ifstream;
#ifndef WIN32
using std::min;
using std::max;
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
using boost::property_tree::ptree; using boost::property_tree::read_json; using boost::property_tree::write_json;
#ifndef SKIP_DATETIME
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
#endif

#ifdef RAYHEADERSMAIN
namespace boost { namespace property_tree { namespace json_parser
{
    // Create necessary escape sequences from illegal characters
    template<>
    std::basic_string<char> create_escapes(const std::basic_string<char> &s)
    {
        std::basic_string<char> result;
        std::basic_string<char>::const_iterator b = s.begin();
        std::basic_string<char>::const_iterator e = s.end();
        while (b != e)
        {
            // This assumes an ASCII superset. But so does everything in PTree.
            // We escape everything outside ASCII, because this code can't
            // handle high unicode characters.
            if (*b == 0x20 || *b == 0x21 || (*b >= 0x23 && *b <= 0x2E) ||
                (*b >= 0x30 && *b <= 0x5B) || (*b >= 0x5D && *b <= 0xFF)  //it fails here because char are signed
                || (*b >= -0x80 && *b < 0 ) ) // this will pass UTF-8 signed chars
                result += *b;
            else if (*b == char('\b')) result += char('\\'), result += char('b');
            else if (*b == char('\f')) result += char('\\'), result += char('f');
            else if (*b == char('\n')) result += char('\\'), result += char('n');
            else if (*b == char('\r')) result += char('\\'), result += char('r');
            else if (*b == char('/')) result += char('\\'), result += char('/');
            else if (*b == char('"'))  result += char('\\'), result += char('"');
            else if (*b == char('\\')) result += char('\\'), result += char('\\');
            else
            {
                const char *hexdigits = "0123456789ABCDEF";
                typedef make_unsigned<char>::type UCh;
                unsigned long u = (std::min)(static_cast<unsigned long>(
                                                 static_cast<UCh>(*b)),
                                             0xFFFFul);
                int d1 = u / 4096; u -= d1 * 4096;
                int d2 = u / 256; u -= d2 * 256;
                int d3 = u / 16; u -= d3 * 16;
                int d4 = u;
                result += char('\\'); result += char('u');
                result += char(hexdigits[d1]); result += char(hexdigits[d2]);
                result += char(hexdigits[d3]); result += char(hexdigits[d4]);
            }
            ++b;
        }
        return result;
    }
} } }
#endif

string WstringToString(wstring in);
#ifdef RAYHEADERSMAIN
string WstringToString(wstring in)
{
#ifdef WIN32
	int len = WideCharToMultiByte(CP_UTF8, 0, in.c_str(), (int)in.length()+1, NULL, 0, 0, 0);
	char *buf = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, in.c_str(), (int)in.length()+1, buf, len, 0, 0);
	string out = buf;
	delete[] buf;
	return out;
#else
	std::setlocale(LC_ALL, "");
	const std::locale locale("");
	typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
	const converter_type& converter = std::use_facet<converter_type>(locale);
	std::vector<char> to(in.length() * converter.max_length());
	std::mbstate_t state;
	const wchar_t* from_next;
	char* to_next;
	const converter_type::result result = converter.out(state, in.data(), in.data() + in.length(), from_next, &to[0], &to[0] + to.size(), to_next);
	if (result == converter_type::ok or result == converter_type::noconv) {
		const std::string s(&to[0], to_next);
		return s;
	}
#endif
	return string("");
}
#endif

string DecodePythonUnicode(string in);
#ifdef RAYHEADERSMAIN
string DecodePythonUnicode(string in)
{
	string out;
	const char *s = in.c_str();
	int escapes=0;
	while(*s) {
		/*const char *u = strstr(s, "\u");
		const char *U = strstr(s, "\U");
		const char *e = u;
		if(u==NULL || U<u)
			e=U;*/
		const char *e = strchr(s, '\\');
		if(e==NULL) {
			out+=s;
			//cout << out << "\n";
			return out;
		}
		out+=string(s, e-s);
		//out+="\\";
		escapes=1;
		if(out.length()>0 && out[out.length()-1]=='\\') {
			escapes++;
		} else
			escapes=1;
		if(escapes%2==1 && (e[1]=='u' || e[1]=='U') ) {
			//out+="\\";
			if(e[1]=='u'&&strlen(e+1)>=5) {
				wchar_t t[2];
				sscanf(e+2, "%4x", (unsigned int*)t);
				out+=WstringToString(wstring(t,1));
				e+=5;
			} else if(e[1]=='U'&&strlen(e+1)>=9) {
				wchar_t t[2];
				sscanf(e+2, "%8x", (unsigned int*)t);
				out+=WstringToString(wstring(t,2));
				e+=9;
			}
		} else
			out+="\\";
		s=e+1;

	}
	out+=s;
	//cout << out << "\n";
	return out;
}
#endif

#ifdef WIN32
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif
//using namespace mysqlpp;

#ifndef NOCURL
#define CURL_STATICLIB
#include <curl/curl.h>
#include <openssl/md5.h>
#endif
#ifdef NOCURL
#define CURL int
#endif

extern CURL *curl;
#ifdef RAYHEADERSMAIN
CURL *curl;
#endif

//#define RAYHEADERSMAIN
#define DEBUGLEVEL ERR
#define DEBUGFILELEVEL INFO
#ifdef RAYHEADERSMAIN
#ifndef DEBUGFILENAME
#define DEBUGFILENAME "../debugout.txt"
#endif
#endif

#include <rayutil.h>
#include <oswrapper.h>
#include <raystringstuff.h>
#include <raycontainers2.h>
#include <rayhashtable.h>
#include <rayheap.h>
#include <rayheapnew.h>
//#include <sockets4.h>

/*#ifdef WIN32
class rayexception : public std::exception
{
public:
	rayexception(const char *s DEBUGARGDEF) : std::exception(s)
	{
	}
};
#else*/
class rayexception// : public std::exception
{
public:
	string d;
	rayexception(const char *s DEBUGARGSDEF);
	//{
	//	DEBUGOUTSTACK(ERR, d, "rayexception()");
	//}

	const char *saywhat( DEBUGARGDEF)
	{
		DEBUGOUTSTACK(ERR, d, "rayexception::what()");
		return d.c_str();
	}

	const char *what( DEBUGARGDEF)
	{
		return d.c_str();
	}
};

//#endif

//#define HASHPRIME 103
#define HASHPRIME 9

//mysqlpp::Connection conn(false);
extern MYSQL * con;
#ifdef RAYHEADERSMAIN
MYSQL * con=NULL;
#endif

string HexMD5(string input);
#ifdef RAYHEADERSMAIN
string HexMD5(string input)
{
	string ret;
#ifndef NOCURL
	unsigned char result[MD5_DIGEST_LENGTH];
	char hex[33];
	hex[32]='\0';
	MD5((const unsigned char *)input.c_str(), input.length(), result);
	for(int i=0;i<MD5_DIGEST_LENGTH;i++) {
		sprintf(&hex[i*2], "%02x", result[i]);
	}
	ret=hex;
#endif
	return ret;
}
#endif

string MysqlEscape(string in, size_t max_len/*=1024*/);
#ifdef RAYHEADERSMAIN
string MysqlEscape(string in, size_t max_len)
{
	if(in.length()>max_len) {
		throw rayexception("MysqlEscape in.length()>max_len" DEBUGARGS);
	}
	char buff[4096];
	assert(in.length()*2+1 < 4096);
	auto len = mysql_real_escape_string(con, buff, in.c_str(), (uint)in.length());
	return string(buff, len);
}
#endif

string HTMLEscape(string in, size_t max_len=128);
#ifdef RAYHEADERSMAIN
string HTMLEscape(string in, size_t max_len)
{
	if(in.length()>max_len) {
		throw rayexception("HTMLEscape in.length()>max_len" DEBUGARGS);
	}
	const char *symbols[]={"\"&quot;", "'&#39;", "&&amp;", "<&lt;", ">&gt;"};
	string out;
	const char *s=in.c_str();
	const char *e=s+in.length();
	while(*s) {
		const char *n=e;
		int a=0;
		for(int i=0;i<5;i++) {
			const char *t = strchr(s,symbols[i][0]);
			if(t && t<n) {
				n=t;
				a=i;
			}
		}
		if(n!=e) {
			out+=string(s,n-s);
			out+=symbols[a]+1;
			s=n+1;
		} else {
			out+=s;
			break;
		}
	}
	return out;
}
#endif

class rstring : public string
{
public:
	operator string()
	{
		return *this;
	}

	rstring() : string(){}
	rstring(const rstring &s) : string(s.c_str()){}
	rstring(const unsigned int s) : string( ToStringCommas(s) ){}
	rstring(const int s) : string( ToStringCommas(s) ){}
	rstring(const float s) : string( ToStringPrecision(s,4) ){}
	rstring(const string s) : string(s){}
	rstring(const char *s) : string((s? s : "")){}
	rstring(const char *s, const unsigned int len) : string((s? s : ""),(s? len : 4)){}

	operator int()
	{
		return ToInt(c_str());
	}

	operator uint()
	{
		return ToUInt(c_str());
	}
};

class CGIVariable : public rstring
{
public:
	rstring name;

	CGIVariable()
	{
	}

	CGIVariable(const char *key) : name(key)
	{
	}

	CGIVariable(const char *key, const char *value) : rstring(value), name(key)
	{
	}

	CGIVariable(const char *key, unsigned int key_len, const char *value, unsigned int value_len) : rstring(value,value_len), name(key,key_len)
	{
	}

	~CGIVariable()
	{
		//cerr << "~\n";
	}

	/*virtual void Transport(CGIVariable *p)
	{
		new(p) CGIVariable(*this);
	}*/

	/*template<class A>
	CGIVariable& operator=(A s)
	{
		this->operator =(s);
		return *this;
	}

	template<>
	CGIVariable& operator=(int i)
	{
		name=ToStringCommas(i);
		return *this;
	}

	template<class A>
	CGIVariable& operator+=(A s)
	{
		name+=s;
		return *this;
	}

	template<class A>
	string operator+(A s)
	{
		return name+s;
	}*/

	static unsigned int Hash(const char *k)
	{
		unsigned int h=0;
		for(;*k;k++)
			h=h*HASHPRIME+*k;
		return h;
	}

	unsigned int Hash()
	{
		return Hash(name.c_str());
	}

	int SortComp(CGIVariable* objB)
	{
		return name != objB->name;
	}

	int SortComp(const char *objB)
	{
		return name != objB;
	}
};

class VarRef;

class CGIVariables : public HashTable<CGIVariable,const char*>
{
private:
	CGIVariables(const CGIVariables &old) : HashTable<CGIVariable,const char*>(8)
	{
		cerr << "WTF!\n";
	}

public:

	CGIVariables& operator=(CGIVariables &&old)
	{
		HashTable::operator=(std::move(old));
		return *this;
	}

	CGIVariables(unsigned int i) : HashTable<CGIVariable,const char*>(i)
	{
	}

	CGIVariables(CGIVariables &&old) : HashTable<CGIVariable,const char*>(std::move(old))
	{
	}

	~CGIVariables()
	{
		//cerr << "~\n";
	}

	VarRef operator[](const char *key);

	string ToString();
};

class VarRef
{
	rstring name;
	CGIVariables *hashtable;
	CGIVariable *var;

public:
	VarRef(CGIVariables *table, CGIVariable *Var, const char *Key) : name(Key)
	{
		hashtable=table;
		var=Var;
	}

	template<class A>
	rstring operator=(A a)
	{
		if(!var)
			var=hashtable->CreateKey<CGIVariable>(name.c_str());

		return *((rstring*)var)=a;
	}

	template<class A>
	rstring operator+=(A a)
	{
		if(!var)
			var=hashtable->CreateKey<CGIVariable>(name.c_str());

		return *var+=a;
	}

	template<class A>
	bool operator==(A a)
	{
		if(var)
			return *var==a;
		return string("")==a;
	}

	template<class A>
	bool operator!=(A a)
	{
		if(var)
			return *var!=a;
		return string("")!=a;
	}

	operator rstring()
	{
		if(var)
			return *var;
		return rstring("");
	}

	string ToString()
	{
		if(var)
			return *var;
		return string("");
	}

	/*operator string()
	{
		return ToString();
	}*/

	operator int()
	{
		if(var)
			return ToInt(var->c_str());
		return 0;
	}

	operator uint()
	{
		if(var)
			return ToUInt(var->c_str());
		return 0;
	}

	operator bool()
	{
		return (bool)(var!=NULL);
	}

	const char *c_str(const char *def="")
	{
		if(var)
			return var->c_str();
		return def;
	}
};

#define foreach(TYPE,P,C) for(TYPE P=(C).GetFirst(); (P); (P)=(C).GetNext((P)))
extern CGIVariables Variables;
#ifdef RAYHEADERSMAIN
VarRef CGIVariables::operator[](const char *key)
{
	char *k=(char*)key;
	CGIVariable *p = Get(k);
	return VarRef(this, p, key);
	//return *p;
}

string CGIVariables::ToString()
{
	rstring out;
	CGIVariables &self = *this;
	foreach(CGIVariable*,p,self)
		out += p->name + " = " + *p + "\n";
	return (string)out;
}

CGIVariables Variables(128);
#endif

/*void URLDecode(char *dest, const char *src, unsigned int srclen)
{
	if(!src)
		return;
	if(!dest)
		return;

	unsigned int a=0;
	for(unsigned int i=0; src[i] && i<srclen;i++,a++)
	{
		dest[a]=src[i];
		if(dest[a]=='%')
		{
			i++;
			if(src[i] == '\0' || i>=srclen)
			{
				dest[a] = '\0';
				break;
			}
			else if(src[i] >= '0' && src[i] <= '9')
				dest[a] = (src[i] - 48)*16;
			else
				dest[a] = (src[i] - 55)*16;
			i++;
			if(src[i] == '\0' || i>=srclen)
			{
				dest[a] = '\0';
				break;
			}
			else if(src[i] >= '0' && src[i] <= '9')
				dest[a] += (src[i] - 48);
			else
				dest[a] += (src[i] - 55);
		}
		else if(dest[a]=='+')
			dest[a]=' ';
	}
	dest[a]='\0';
}*/

string uri_escape(const char *in, size_t max_len=1024, CURL *c=curl);
string uri_unescape(const char *in, size_t max_len=1024, CURL *c=curl);
size_t curlcallback( char *ptr, size_t size, size_t nmemb, void *userdata);

#ifdef RAYHEADERSMAIN
size_t curlcallback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	string *str = (string*)userdata;
	(*str) += string(ptr, nmemb);
	//cout << "size=="<<size<<", nmemb=="<<nmemb<<"\n";
	return size*nmemb;
}

string uri_escape(const char *in, size_t max_len, CURL *c)
{
	size_t len = strlen(in);
	if(len>max_len) {
		throw rayexception("uri_escape len>max_len" DEBUGARGS);
	}
	string out;
#ifndef NOCURL
	char *buf = curl_easy_escape(curl, in, (uint)len);
	out=buf;
	curl_free(buf);
#else
	out=in;
#endif
	return out;
}

string uri_unescape(const char *in, size_t max_len, CURL *c)
{
	size_t len = strlen(in);
	if(len>max_len) {
		throw rayexception("uri_unescape len>max_len" DEBUGARGS);
	}
	string out;
#ifndef NOCURL
	char *buf = curl_easy_unescape(curl, in, (uint)len, 0);
	out=buf;
	curl_free(buf);
#else
	out=in;
#endif
	return out;
}
#endif

class MysqlTransaction
{
public:
	bool in_trans;

	MysqlTransaction()
	{
		in_trans=false;
	}

	~MysqlTransaction()
	{
		commit();
	}

	int start()
	{
		if(in_trans==true) return 0;
		if(con==NULL) {
			cerr << "start transaction failed, con==NULL\n";
			return 1;
		}
		int ires = mysql_query(con,"start transaction");
		if(ires) {
			cerr << "start transaction failed "<<mysql_error(con)<<"\n";
			return ires;
		}
		in_trans=true;
		return 0;
	}

	int commit()
	{
		if(in_trans==false) return 0;
		if(con==NULL) {
			cerr << "commit failed, con==NULL\n";
			return 1;
		}
		int ires = mysql_query(con, "commit");
		if(ires) {
			cerr << "commit failed "<<mysql_error(con)<<"\n";
			return ires;
		}
		in_trans=false;
		return 0;
	}
};
extern MysqlTransaction mysqltrans;
#ifdef RAYHEADERSMAIN
MysqlTransaction mysqltrans;
#endif

class RayCGI
{
public:
	char postdata[8192];
	char get[8192];
	char cookie[8192];
	unsigned int postlen;

	static void ReadVars(const char *src, CGIVariables &Variables, const char assign='=', const char delim='&')
	{
		const char *k=src;
		const char *e=NULL;
		const char *v=src;
		DEBUGARG;

		while(v)
		{
			v=NULL;
			e=strchr(k,assign);
			if(e)
			{
				v=strchr(e,delim);
				if(v)
				{
					for(;Is_Whitespace(*k) && k<e-1;k++);
					string key = string(k,(unsigned int)(e-k));
					string val = string(e+1, (unsigned int)(v-e)-1);
					key=uri_unescape(key.c_str());
					val=uri_unescape(val.c_str());
					CGIVariable cgiv(key.c_str(), val.c_str());
					Variables.Create( cgiv );
				}
				else
				{
					for(;Is_Whitespace(*k) && k<e-1;k++);
					string key = string(k,(unsigned int)(e-k));
					string val = string(e+1, strlen(e+1));
					key=uri_unescape(key.c_str());
					val=uri_unescape(val.c_str());
					CGIVariable cgiv(key.c_str(), val.c_str());
					Variables.Create( cgiv );
				}
			}
			else
				return;
			k=v+1;
		}
	}

	RayCGI()
	{
		DEBUGARG;
		//postdata=NULL;
		//RaySleep(15000 *1000);
#ifndef NOCURL
		curl_global_init(CURL_GLOBAL_DEFAULT);
 
		curl = curl_easy_init();
		if(!curl) {
			cerr << "Could not init curl!\n";
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlcallback);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
#endif
		char *content_type = getenv("CONTENT_TYPE");
		if(content_type) DEBUGOUT(ERR,content_type,"post content type");

		char *gete = getenv("QUERY_STRING");
		postlen = (unsigned int)ToInt(getenv("CONTENT_LENGTH"));

		char *cookiee = getenv("HTTP_COOKIE");
		//char *cookie = NULL;
		unsigned int cookielen = 0;
		if(cookiee)
		{
			cookielen=(uint)strlen(cookiee);
			//cookie=new char[cookielen+1];
		}

		//char *poste = NULL;
		//postdata=NULL;
		/*if(postlen>0 && postlen<10*1024*1024)
		{
			postdata = new char[postlen+1];
			fread(postdata, 1, postlen, stdin);
			postdata[postlen] = '\0';
			//post = new char[postlen+1];
		}*/
		
		get[0]={0};
		/*URLDecode(get, gete, 8192);
		URLDecode(post, poste, postlen);
		URLDecode(cookie, cookiee, cookielen);*/
		
		if(gete)
			strncpy(get, gete, 8191);
		//if(poste)
		//	strncpy(post, poste, postlen);
		if(cookiee)
			strncpy(cookie, cookiee, min(cookielen+1,8191u));
		get[8191]='\0';
		//if(post)
			//post[postlen]='\0';
		//if(cookie)
		cookie[cookielen]='\0';
		//return;
		ReadVars(get, Variables);

		if(postlen>0 && postlen<8192 && strcmp(content_type,"application/x-www-form-urlencoded")==0) {
			//postdata = new char[postlen+1];
			fread(postdata, 1, postlen, stdin);
			postdata[postlen] = '\0';
			ReadVars(postdata, Variables);
		}

		ReadVars(cookie, Variables, '=', ';');
		//return;
		//delete[] post;
		//delete[] poste;
		//delete[] cookie;
	}

	~RayCGI()
	{
		DEBUGARG;
#ifndef NOCURL
		curl_easy_cleanup(curl);
		curl_global_cleanup();
#endif
		if(con) {
			mysqltrans.commit();
			mysql_close(con);
			con=NULL;
		}
		//delete[] postdata;
	}
};

extern RayCGI raycgi;
string MakeHttpsRequest(string url, bool cache=true, CURL *c=curl);
int ConnectToMysql(const char *credsfile="/home/ray/mysqlcreds.txt");
#ifdef RAYHEADERSMAIN
RayCGI raycgi;

string MakeHttpsRequest(string url, bool cache, CURL *c)
{
	DEBUGARG;
	string str;
#ifndef NOCURL
	string orig_url = url;
	uint start = GetMilliCount();
	if(cache_server==false && cache==true) {
		url="http://127.0.0.1:8081/?url="+uri_escape(url.c_str());
	}

	struct curl_slist *slist=NULL;
	slist = curl_slist_append(slist, "Accept-Charset:utf-8");
	//curl_easy_setopt(c, CURLOPT_HEADER, 1);
	curl_easy_setopt(c, CURLOPT_HTTPHEADER, slist);
 
	curl_easy_setopt(c, CURLOPT_WRITEDATA, (void*)&str);
	curl_easy_setopt(c, CURLOPT_URL, url.c_str());
	CURLcode res = curl_easy_perform(c);
	if(res) {
		if(BDEBUG) {
			cout << res << " " << curl_easy_strerror(res) << "<br>\n";
		}
		cerr << url << " = " << orig_url << " = " << res << " " << curl_easy_strerror(res) << "\n";
		Variables["errors"] += url + " = " + orig_url + " = " + ToString(res) + " " + curl_easy_strerror(res) + "<br>\n";
	}
	//cout << str << "\n";
	curl_slist_free_all(slist);
	uint duration = GetMilliSpan(start);
	if(cache_server==false && duration>2000) {
		cerr << url << " took "<<duration<<" ms\n";
	}
#endif
	return str;
}

int ConnectToMysql(const char *credsfile)
{
	ifstream creds(credsfile);
	string host, db, username, password;
	if (creds.good() == false) {
		DEBUGOUT(ERR, credsfile, "could not open");
		return 0;
	}
	getline(creds, host);
	getline(creds, username);
	getline(creds, password);
	getline(creds, db);
	//cerr << username << "@"<<host<<" "<<db<<"\n";

	con = mysql_init(con);
	con = mysql_real_connect(con, host.c_str(), username.c_str(), password.c_str(), db.c_str(), 0, 0, 0);
	if(con)
		return 1;
	else {
		cerr << "Could not connect to DB!\n";
		Variables["errors"] += "Could not connect to DB<br>\n";
		DEBUGOUT(ERR, host, "");
		DEBUGOUT(ERR, username, "");
		DEBUGOUT(ERR, db, "");
		return 0;
	}
}
#endif

class testostream : public std::basic_ostream< char, std::char_traits< char > >
{
public:
	//string stringbuf;
	std::stringbuf stringbuf;

	testostream() : std::basic_ostream< char, std::char_traits< char > >(&stringbuf), stringbuf()
	{
	}

	string str()
	{
		return stringbuf.str();
	}
};

#ifndef DEBUGQUERIES
#define DEBUGQUERIES 0
#endif

#define QueryOrDie( A, B ) if(DEBUGQUERIES) { cerr << "Debug output, query ("<< (B).str() << ")"; } if( ((A) = (B).store()) == false) { cerr << "failed query! " /*<< (B).str()*/ << conn.error(); exit(1); }
#define QueryOrLog( A, B ) if(DEBUGQUERIES) { cerr << "Debug output, query ("<< (B).str() << ")"; } if( ((A) = (B).store())==false) { cerr << "failed query! " /*<< (B).str()*/ << conn.error(); }

#define QueryExecOrLog( A, B ) if(DEBUGQUERIES) { cerr << "Debug output, query execute ("<< (B).str() << ")"; } if( ((A) = (B).execute())==false) { cerr << "failed to execute query! " /*<< (B).str()*/ << conn.error(); } if(DEBUGQUERIES) { cerr << (A).info(); string s = "Rows affected = " + ToString((A).rows()) + ", insert_id = " + ToString((A).insert_id()); cerr << s; }
#define QueryExecOrDie( A, B ) if(DEBUGQUERIES) { cerr << "Debug output, query execute ("<< (B).str() << ")"; } if( ((A) = (B).execute())==false) { cerr << "failed to execute query! " /*<< (B).str()*/ << conn.error(); exit(1); } if(DEBUGQUERIES) { cerr << (A).info(); string s = "Rows affected = " + ToString((A).rows()) + ", insert_id = " + ToString((A).insert_id()); cerr << s; }

#ifdef RAYHEADERSMAIN
rayexception::rayexception(const char *s DEBUGARGSDEF) : d(s)
{
	string vars;
	foreach(CGIVariable*,p,Variables)
			vars += p->name + "(" + ToString(p->Hash()%Variables.Slots) + ") = " + *p+"\n";
	DEBUGOUT(ERR, vars, "");
	DEBUGOUTSTACK(ERR, d, "rayexception()");
}
#endif
