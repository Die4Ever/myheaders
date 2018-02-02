#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <memory>
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
#include <math.h>
#include <algorithm>
#include <thread>
#include <array>

using std::string;
using std::wstring;
using std::vector;
using std::array;
using std::stringstream;
using std::cerr;
using std::min;
using std::max;
using std::abs;

string WstringToString(wstring in);
string HexMD5(string input);
//string MysqlEscape(string in, size_t max_len);
string HtmlEscape(string in, size_t max_len);

#ifndef DEBUGLEVEL
#define DEBUGLEVEL WARN
#endif
#ifndef DEBUGFILELEVEL
#define DEBUGFILELEVEL INFO
#endif
#include <oswrapper2.h>
#include <rayutil.h>

class rayexception
{
public:
	string d;
	rayexception(const char *s DEBUGARGSDEF);
	//rayexception(string &s DEBUGARGSDEF);
#ifdef DEBUGLOG
	vector< CALLERBASE > trace;
#endif

	string saywhat(DEBUGARGDEF)
	{
		DEBUGOUTSTACK(ERR, d, "rayexception::what()");
		return what(DEBUGARG);
	}

	string what(DEBUGARGDEF);
};

#ifdef USECURL
#define CURL_STATICLIB
#include <curl/curl.h>
#include <openssl/md5.h>
#endif

#ifdef USEJSON
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
using boost::property_tree::ptree; using boost::property_tree::read_json; using boost::property_tree::write_json;
#ifndef SKIP_DATETIME
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
#endif

#ifdef RAYHEADERSMAIN
namespace boost {
	namespace property_tree {
		namespace json_parser
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
						|| (*b >= -0x80 && *b < 0)) // this will pass UTF-8 signed chars
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
		}
	}
}
#endif

string DecodePythonUnicode(string in);
#ifdef RAYHEADERSMAIN
string DecodePythonUnicode(string in)
{
	string out;
	const char *s = in.c_str();
	int escapes = 0;
	while (*s) {
		/*const char *u = strstr(s, "\u");
		const char *U = strstr(s, "\U");
		const char *e = u;
		if(u==NULL || U<u)
		e=U;*/
		const char *e = strchr(s, '\\');
		if (e == NULL) {
			out += s;
			//cout << out << "\n";
			return out;
		}
		out += string(s, e - s);
		//out+="\\";
		escapes = 1;
		if (out.length()>0 && out[out.length() - 1] == '\\') {
			escapes++;
		}
		else
			escapes = 1;
		if (escapes % 2 == 1 && (e[1] == 'u' || e[1] == 'U')) {
			//out+="\\";
			if (e[1] == 'u'&&strlen(e + 1) >= 5) {
				wchar_t t[2];
				sscanf(e + 2, "%4x", (unsigned int*)t);
				out += WstringToString(wstring(t, 1));
				e += 5;
			}
			else if (e[1] == 'U'&&strlen(e + 1) >= 9) {
				wchar_t t[2];
				sscanf(e + 2, "%8x", (unsigned int*)t);
				out += WstringToString(wstring(t, 2));
				e += 9;
			}
		}
		else
			out += "\\";
		s = e + 1;

	}
	out += s;
	//cout << out << "\n";
	return out;
}
#endif
#endif

#include <raystringstuff2.h>
#include <raycontainers2.h>
#include <rayhashtable.h>
#include <rayheap.h>
#include <rayheapnew.h>

#ifdef WIN32
#include <winsock2.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

class RMYSQL_CELL
{
public:
	const char *d;
	operator int()
	{
		return ToInt(d);
	}

	operator uint()
	{
		return ToUInt(d);
	}

	operator uint64_t()
	{
		return ToUInt64(d);
	}

	/*operator string()
	{
		if (d) return string(d);
		return string();
	}*/

	string ToString()
	{
		if (d) return string(d);
		return string();
	}

	const char * cstr()
	{
		return d;
	}
	/*operator const char*()
	{
		return d;
	}*/
};
/*class RMYSQL_ROW
{
public:
	RMYSQL_CELL *cells;
};*/
typedef RMYSQL_CELL* RMYSQL_ROW;

class MysqlCon
{
public:
	bool connected;
	bool in_trans;
	MYSQL con;
	MYSQL_RES *res;

	MysqlCon()
	{
		//con = NULL;
		in_trans = connected = false;
		res = NULL;
	}

	~MysqlCon()
	{
		free_result();
		Commit();
		if (connected) {
			mysql_close(&con);
			connected = false;
			//con=NULL;
		}
	}

	int StartTransaction()
	{
		if (in_trans) return 0;
		if (connected == false) return 1;

		int ires = mysql_query(&con, "start transaction");
		if (ires) {
			cerr << "start transaction failed " << mysql_error(&con) << "\n";
			return ires;
		}
		in_trans = true;
		return 0;
	}

	int Commit()
	{
		if (in_trans == false) return 0;
		if (connected == false) return 1;

		int ires = mysql_query(&con, "commit");
		if (ires) {
			cerr << "commit transaction failed " << mysql_error(&con) << "\n";
			return ires;
		}
		in_trans = false;
		return 0;
	}

	int ConnectToMysql(const char *credsfile)
	{
		std::ifstream creds(credsfile);
		if (creds.good() == false) {
			cerr << credsfile << " not found!\n";
			throw rayexception("could not connect to DB!" DEBUGARGS);
			return 0;
			//exit(1);
		}
		string host, db, username, password;
		getline(creds, host);
		getline(creds, username);
		getline(creds, password);
		getline(creds, db);
		//cerr << username << "@"<<host<<" "<<db<<"\n";

		mysql_init(&con);
		if (mysql_real_connect(&con, host.c_str(), username.c_str(), password.c_str(), db.c_str(), 0, 0, 0)) {
			connected = true;
			my_bool opt = true;
			mysql_options(&con, MYSQL_OPT_COMPRESS, &opt);
			//cerr << "Connected to db!\n";
			//exit(0);
			return 1;
		}
		else {
			connected = false;
			cerr << "Could not connect to DB!\n";
			cerr << mysql_error(&con) << "\n";
			throw rayexception("could not connect to DB!" DEBUGARGS);
			//exit(1);
			return 0;
		}
	}

	void free_result()
	{
		if (res) {
			mysql_free_result(res);
			res = NULL;
		}
	}

	MYSQL_RES * select(const char * query DEBUGARGSDEF)
	{
		free_result();
		int ires = mysql_query(&con, query);
		if (ires) {
			string q = query;
			if(q.length()>5000) q=q.substr(0, 5000);
			DEBUGOUT(FATAL, q, "");
			throw rayexception(mysql_error(&con) DEBUGARGS);
		}
		res = mysql_store_result(&con);
		return res;
	}

	RMYSQL_ROW fetch_row(MYSQL_RES *res)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		return (RMYSQL_ROW)row;
	}

	unsigned __int64 insert(const char * query DEBUGARGSDEF)
	{
		DEBUGOUT(TRACE,query,"");
		int ires = mysql_query(&con, query);
		if (ires) {
			string q = query;
			if (q.length()>5000) q=q.substr(0, 5000);
			DEBUGOUT(FATAL, q, "");
			throw rayexception(mysql_error(&con) DEBUGARGS);
		}
		return (unsigned __int64)mysql_insert_id(&con);
	}

	void update(const char * query DEBUGARGSDEF)
	{
		DEBUGOUT(TRACE,query,"");
		int ires = mysql_query(&con, query);
		if (ires) {
			string q = query;
			if (q.length()>5000) q=q.substr(0, 5000);
			DEBUGOUT(FATAL, q, "");
			throw rayexception(mysql_error(&con) DEBUGARGS);
		}
	}

	void delete_query(const char * query DEBUGARGSDEF)
	{
		DEBUGOUT(TRACE,query,"");
		int ires = mysql_query(&con, query);
		if (ires) {
			string q = query;
			if (q.length()>5000) q=q.substr(0, 5000);
			DEBUGOUT(FATAL, q, "");
			throw rayexception(mysql_error(&con) DEBUGARGS);
		}
	}

	string escape(string in, size_t max_len)
	{
		if (in.length()>max_len) {
			throw rayexception("MysqlEscape in.length()>max_len" DEBUGARGS);
		}
		char buff[16384];
		assert(in.length() * 2 + 1 < 16384);
		auto len = mysql_real_escape_string(&con, buff, in.c_str(), (uint)in.length());
		return string(buff, len);
	}
};
extern MysqlCon mysqlcon;

#define HASHPRIME 9

class rstring : public string
{
public:
	operator string()
	{
		return *this;
	}

	rstring() : string(){}
	rstring(const rstring &s) : string(s.c_str()){}
	rstring(const unsigned int s) : string(ToStringCommas(s)){}
	rstring(const int s) : string(ToStringCommas(s)){}
	rstring(const float s) : string(ToStringPrecision(s, 4)){}
	rstring(const string s) : string(s){}
	rstring(const char *s) : string((s ? s : "")){}
	rstring(const char *s, const unsigned int len) : string((s ? s : ""), (s ? len : 4)){}

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

	CGIVariable(const char *key, unsigned int key_len, const char *value, unsigned int value_len) : rstring(value, value_len), name(key, key_len)
	{
	}

	~CGIVariable()
	{
	}

	static unsigned int Hash(const char *k)
	{
		unsigned int h = 0;
		for (; *k; k++)
			h = h*HASHPRIME + *k;
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

class CGIVariables : public HashTable<CGIVariable, const char*>
{
private:
	CGIVariables(const CGIVariables &old) : HashTable<CGIVariable, const char*>(8)
	{
		cerr << "WTF!\n";
	}

public:

	CGIVariables& operator=(CGIVariables &&old)
	{
		HashTable::operator=(std::move(old));
		return *this;
	}

	CGIVariables(unsigned int i) : HashTable<CGIVariable, const char*>(i)
	{
	}

	CGIVariables(CGIVariables &&old) : HashTable<CGIVariable, const char*>(std::move(old))
	{
	}

	~CGIVariables()
	{
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
		hashtable = table;
		var = Var;
	}

	template<class A>
	rstring operator=(A a)
	{
		if (!var)
			var = hashtable->CreateKey<CGIVariable>(name.c_str());

		return *((rstring*)var) = a;
	}

	template<class A>
	rstring operator+=(A a)
	{
		if (!var)
			var = hashtable->CreateKey<CGIVariable>(name.c_str());

		return *var += a;
	}

	template<class A>
	bool operator==(A a)
	{
		if (var)
			return *var == a;
		return string("") == a;
	}

	template<class A>
	bool operator!=(A a)
	{
		if (var)
			return *var != a;
		return string("") != a;
	}

	operator rstring()
	{
		if (var)
			return *var;
		return rstring("");
	}

	string ToString()
	{
		if (var)
			return *var;
		return string("");
	}

	operator int()
	{
		if (var)
			return ToInt(var->c_str());
		return 0;
	}

	operator uint()
	{
		if (var)
			return ToUInt(var->c_str());
		return 0;
	}

	operator bool()
	{
		return (bool)(var != NULL);
	}

	const char *c_str(const char *def = "")
	{
		if (var)
			return var->c_str();
		return def;
	}

	size_t length()
	{
		if (var)
			return var->length();
		return 0;
	}
};

#define foreach(TYPE,P,C) for(TYPE P=(C).GetFirst(); (P); (P)=(C).GetNext((P)))
extern CGIVariables vars;
extern CGIVariables postvars;

class RayCGI
{
public:
	char postdata[8192];
	char get[8192];
	char cookie[8192];
	unsigned int postlen;

	static void ReadVars(const char *src, CGIVariables &Variables, const char assign = '=', const char delim = '&')
	{
		const char *k = src;
		const char *e = NULL;
		const char *v = src;
		DEBUGARG;

		while (v)
		{
			v = NULL;
			e = strchr(k, assign);
			if (e)
			{
				v = strchr(e, delim);
				if (v)
				{
					for (; Is_Whitespace(*k) && k<e - 1; k++);
					string key = string(k, (unsigned int)(e - k));
					string val = string(e + 1, (unsigned int)(v - e) - 1);
#ifdef USECURL
					key = uri_unescape(key.c_str());
					val = uri_unescape(val.c_str());
#endif
					CGIVariable cgiv(key.c_str(), val.c_str());
					Variables.Create(cgiv);
				}
				else
				{
					for (; Is_Whitespace(*k) && k<e - 1; k++);
					string key = string(k, (unsigned int)(e - k));
					string val = string(e + 1, strlen(e + 1));
#ifdef USECURL
					key = uri_unescape(key.c_str());
					val = uri_unescape(val.c_str());
#endif
					CGIVariable cgiv(key.c_str(), val.c_str());
					Variables.Create(cgiv);
				}
			}
			else
				return;
			k = v + 1;
		}
	}

	RayCGI()
	{
		DEBUGARG;
#ifdef USECURL
		curl_global_init(CURL_GLOBAL_DEFAULT);

		curl = curl_easy_init();
		if (!curl) {
			cerr << "Could not init curl!\n";
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlcallback);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
#endif
		char *content_type = getenv("CONTENT_TYPE");
		if (content_type) DEBUGOUT(ERR, content_type, "post content type");

		char *gete = getenv("QUERY_STRING");
		postlen = (unsigned int)ToInt(getenv("CONTENT_LENGTH"));

		char *cookiee = getenv("HTTP_COOKIE");
		unsigned int cookielen = 0;
		if (cookiee)
		{
			cookielen = (uint)strlen(cookiee);
		}

		get[0] = { 0 };

		if (gete)
			strncpy(get, gete, 8191);
		if (cookiee)
			strncpy(cookie, cookiee, min(cookielen + 1, 8191u));
		get[8191] = '\0';
		cookie[cookielen] = '\0';
		vars["_GET"]=get;
		ReadVars(get, vars);

		/*if (postlen > 0) {
			fread(postdata, 1, postlen, stdin);
			postdata[postlen] = '\0';
			vars["post-data"] = postdata;
			vars["content-type"] = content_type;
		}*/
		if (postlen>0 && postlen<8192 && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
			fread(postdata, 1, postlen, stdin);
			postdata[postlen] = '\0';
			ReadVars(postdata, postvars);
		}

		ReadVars(cookie, vars, '=', ';');
	}

	~RayCGI()
	{
		DEBUGARG;
#ifdef USECURL
		curl_easy_cleanup(curl);
		curl_global_cleanup();
#endif
	}
};

extern RayCGI raycgi;
#ifdef RAYHEADERSMAIN
MysqlCon mysqlcon;

string HtmlEscape(string in, size_t max_len)
{
	if (in.length()>max_len) {
		throw rayexception("HTMLEscape in.length()>max_len" DEBUGARGS);
	}
	const char *symbols[] = { "\"&quot;", "'&#39;", "&&amp;", "<&lt;", ">&gt;" };
	string out;
	const char *s = in.c_str();
	const char *e = s + in.length();
	while (*s) {
		const char *n = e;
		int a = 0;
		for (int i = 0; i<5; i++) {
			const char *t = strchr(s, symbols[i][0]);
			if (t && t<n) {
				n = t;
				a = i;
			}
		}
		if (n != e) {
			out += string(s, n - s);
			out += symbols[a] + 1;
			s = n + 1;
		}
		else {
			out += s;
			break;
		}
	}
	return out;
}

string HexMD5(string input)
{
	string ret;
#ifdef USECURL
	unsigned char result[MD5_DIGEST_LENGTH];
	char hex[33];
	hex[32] = '\0';
	MD5((const unsigned char *)input.c_str(), input.length(), result);
	for (int i = 0; i<MD5_DIGEST_LENGTH; i++) {
		sprintf(&hex[i * 2], "%02x", result[i]);
	}
	ret = hex;
#else
	ret = "no md5!";
#endif
	return ret;
}

VarRef CGIVariables::operator[](const char *key)
{
	char *k = (char*)key;
	CGIVariable *p = Get(k);
	return VarRef(this, p, key);
	//return *p;
}

string CGIVariables::ToString()
{
	rstring out;
	CGIVariables &self = *this;
	foreach(CGIVariable*, p, self)
		out += p->name + " = " + *p + "\n";
	return (string)out;
}

CGIVariables vars(128);
CGIVariables postvars(8);

RayCGI raycgi;
rayexception::rayexception(const char *s DEBUGARGSDEF) : d(s)
{
#ifdef DEBUGLOG
	trace=stacktrace;
#endif
	string svars = "\n";
	foreach(CGIVariable*, p, vars)
		svars += p->name + "(" + ToString(p->Hash() % vars.Slots) + ") = " + *p + "\n";
	svars += "\n";
	DEBUGOUT(ERR, svars, "");
	DEBUGOUTSTACK(ERR, d, "rayexception()");
}

string rayexception::what(DEBUGARGDEF)
{
	string ret=d;
#ifdef DEBUGLOG
	ret+="\n";
	for(auto &s : trace) {
		ret+=string(s.file)+"::"+s.func+"::"+ToString(s.line)+" --- \n";
	}
#endif
	return ret;
}

/*rayexception::rayexception(string &s DEBUGARGSDEF) : d(s)
{
	string vars;
	foreach(CGIVariable*, p, Variables)
		vars += p->name + "(" + ToString(p->Hash() % Variables.Slots) + ") = " + *p + "\n";
	DEBUGOUT(ERR, vars, "");
	DEBUGOUTSTACK(ERR, d, "rayexception()");
}*/
#endif

#define DefStringType(A,B) class A {\
	string d; \
public:\
	A() {}\
	A(const char *n) : d(n) {}\
	A(string n) : d(n) {}\
	A(string &n) : d(n) {}\
	const char *c_str() { return d.c_str(); }\
	string ToString() { return d; }\
	friend std::ostream& operator << (std::ostream &out, A &b) { out << b.d; return out; }\
	B\
}

#define DefFixedStringType(A,B,C) class A {\
	char d[(B)]; \
public:\
	A() { d[0] = 0; }\
	A(const char *n) { strcpy(d, n); }\
	A(string n) { strcpy(d, n.c_str()); }\
	A(string &n) { strcpy(d, n.c_str()); }\
	const char *c_str() { return d; }\
	string ToString() { return string(d); }\
	friend std::ostream& operator << (std::ostream &out, A &b) { out << b.d; return out; }\
	C\
}
