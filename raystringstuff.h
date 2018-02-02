#pragma once
#ifdef WIN32
#define sscanf sscanf_s
#endif

__inline int Is_Alphanumeric(char c)
{
	if( (c >= 'A' && c <= 'Z') || ( c >= 'a' && c <= 'z') )
		return 1;
	if( c >= '0' && c <= '9' )
		return 2;
	return 0;
}

__inline int Is_Whitespace(char c)
{
	if( (c > 7 && c < 14) || c == ' ')
		return 1;
	return 0;
}

__inline string StripWhitespace(const string &s)
{
	size_t start = 0;
	size_t end = s.length();
	for (size_t i = 0; i<s.length(); i++) {
		if (!Is_Whitespace(s[i])) {
			start = i;
			break;
		}
	}
	for (size_t i = s.length() - 1; i<s.length(); i--) {
		if (!Is_Whitespace(s[i])) {
			end = i + 1;
			break;
		}
	}
	return s.substr(start, end - start);
}

__inline string StripRepeatWhitespace(const string &s)
{
	string r;
	bool in_white = false;
	r.reserve(s.capacity());
	for (size_t i = 0; i<s.length(); i++) {
		if (!Is_Whitespace(s[i])) {
			r += s[i];
			in_white = false;
		} else if (in_white == false) {
			r += s[i];
			in_white = true;
		}
	}
	return r;
}

template <class A>
string ToString(A a)
{
	stringstream s;
	s << a;
	return s.str();
}

//string ToString(double a);
string ToString(double a, int prec);
string URLDecode(string in);
string jsonescape(string in, size_t max_len);
#ifdef RAYHEADERSMAIN

string jsonescape(string in, size_t max_len)
{
	/*if (in.length()>max_len) {
		throw rayexception("jsonescape in.length()>max_len" DEBUGARGS);
	}*/
	if(in.length()>max_len) {
		throw std::exception();
	}
	string out;
	for(uint i=0;i<in.length();i++) {
		if(in[i]=='\r' || in[i]=='\n' || in[i]=='\t') {
			if(in[i]=='\r') out+="\\r";
			if(in[i]=='\n') out+="\\n";
			if(in[i]=='\t') out+="\\t";
			continue;
		}
		if(in[i]=='\\' || in[i]=='"') out+='\\';
		out+=in[i];
	}
	return out;
}

int HexCharToValue(char c)
{
	int r=0;
	if(c<='9') r=(int)(c-'0');
	else r=(int)(c-'A')+10;
	return r;
}

string URLDecode(string in)
{
	string ret;
	for(uint i=0;i<in.length();i++) {
		if(in[i]=='%') {
			char a=in[i+1];
			char b=in[i+2];
			char c=0;
			c=(char) (HexCharToValue(a)*16+HexCharToValue(b));
			ret+=c;
			i+=2;
		} else ret+=in[i];
	}
	return ret;
}

/*string ToString(double a)
{
	char buff[64];
	sprintf(buff, "%0.6lf", a);
	string ret=buff;
	return ret;
}*/
string ToString(double a, int prec)
{
	char buff[64];
	sprintf(buff, ("%0."+ToString(prec)+"lf").c_str(), a);
	string ret=buff;
	//if(prec==3 && a>0.0 && ret=="0.000") ret=">0.000";
	return ret;
}
#endif

struct my_facet : public std::numpunct<char> {
	explicit my_facet(size_t refs = 0) : std::numpunct<char>(refs) {}
	virtual char do_thousands_sep() const { return ','; }
	virtual std::string do_grouping() const { return "\003"; }
};

template<class A>
string ToStringCommas(A a)
{
	stringstream s;
	std::locale global;
	std::locale withgroupings(global, new my_facet);
	s.imbue(withgroupings);
	s << a;
	return s.str();
}

#ifndef RAYHEADERSMAIN
int MatchToken(char *buff, char **tokens, int bufflen=0);
int Find(char *str, char *str2);
unsigned char* ToHex(unsigned char *data, int Len, unsigned char *buff);
#ifdef MD5
char* Md5Hex(char *data, char *buff);
#endif

int wFind(wchar_t *str, wchar_t *str2);

template <>
string ToString<char*>(char *c);
template <>
string ToString<const char*>(const char *c);

string ToStringPrecision(float f, int prec);
unsigned int TypoHash(const char *a);
bool SplitBy(const char *a, char c, vector<string> &strings);
bool SplitBy(char *a, char c, vector<char*> &strings);
bool SplitByTrimmed(const char *a, char c, vector<string> &strings);
vector<string> SuperSplit(const char *source, const char *format);
#else

int MatchToken(char *buff, char **tokens, int bufflen=0)
{
	//int a=0;
	//int e=0;
	//int i=0;
	if(bufflen == 0)
		bufflen = (int)strlen(buff);
	int tokenlen;

	for(int e=0;tokens[e] != 0;e++)
	{
		tokenlen = (int)strlen(tokens[e]);
		if(tokenlen < bufflen)
		{
			if( memcmp(buff, tokens[e], tokenlen)==0 )
				return e;
		}
	}
	return -1;
}

int Find(const char *str, const char *str2)
{
	const char *p;
	if( (p = strstr(str, str2)) )
	{
		return (int)(p-str);
	}
	return -1;
}

unsigned char* ToHex(unsigned char *data, int Len, unsigned char *buff)
{
	int i=0;
	for(i=0;i<Len;i++)
	{
		buff[i*2] = data[i] / 16;
		if(buff[i*2]<10)
			buff[i*2] += 48;
		else
			buff[i*2] += 87;

		buff[i*2+1] = data[i] % 16;
		if(buff[i*2+1]<10)
			buff[i*2+1] += 48;
		else
			buff[i*2+1] += 87;
	}
	buff[i*2] = '\0';
	return buff;
}

#ifdef MD5
char* Md5Hex(char *data, char *buff)
{
	int i;
	unsigned char md5buffer[16];
	//cout << data << "\n";
	md5_buffer(data, strlen(data), md5buffer);

	for(i=0;i<16;i++)
	{
		buff[i*2] = md5buffer[i] / 16;
		if(buff[i*2]<10)
			buff[i*2] += 48;
		else
			buff[i*2] += 87;

		buff[i*2+1] = md5buffer[i] % 16;
		if(buff[i*2+1]<10)
			buff[i*2+1] += 48;
		else
			buff[i*2+1] += 87;
	}
	buff[i*2] = '\0';
	return buff;
}
#endif

int wFind(wchar_t *str, wchar_t *str2)
{
	int i=0,e=0;
	for(;str[i] != L'\0';i++)
	{
		for(e=0;str[i+e] == str2[e] || str2[e] == L'\0';e++)
		{
			if(str2[e] == L'\0')
			{
				return i;
			}
		}
	}
	return -1;
}

template <>
string ToString<char*>(char *c)
{
	if(c)
		return string(c);
	else
		return string();
}

template <>
string ToString<const char*>(const char *c)
{
	if (c)
		return string(c);
	else
		return string();
}

string ToStringPrecision(float f, int prec)
{
	stringstream s;
	stringstream s2;
	s << ToStringCommas((int)f);
	if(prec>0)
		s << '.';
	s2 << (int)((f-(int)f)*100000000.0);

	string str = s2.str();
	int i=(int)str.length()-1;
	for(;i>=0 && str[i]=='0';i--);
	str.resize(i+1);

	s << str.substr(0, prec);

	return s.str();
}

unsigned int TypoHash(const char *a)
{
	unsigned int a1;
	if(!a)
		return 0;

	unsigned char lastc=0;
	unsigned char c = *a;

	if(c>='A' && c<='Z')
		c-='A';
	else if(c>='a' && c<='z')
		c-='a';

	a1=c<<12;
	unsigned int counter=0;
	for(a++;*a;a++)
	{
		lastc=c;
		c = *a;
		if(c>='A' && c<='Z')
			c-='A';
		else if(c>='a' && c<='z')
			c-='a';
		else
		{
			c=lastc;
			continue;
		}
		a1+=c;
		counter++;
	}

	a1+= ((unsigned int)c)<<17;

	a1 = (a1&0xffffff)+(counter<<24);

	return a1;
}

__inline void SubStr(const char *str, const char *start, const char *end, char *buff, unsigned int len, bool inclusive)
{
	buff[0] = '\0';
	const char *s = strstr(str, start);
	if(s)
	{
		if(!inclusive)
			s += strlen(start);
		const char *e = strstr(s, end);
		if(e)
		{
			if(inclusive)
				e += strlen(end);
			unsigned int slen = min(len-1, (unsigned int)(e-s));
			memcpy(buff, s, slen);
			buff[slen] = '\0';
		}
	}
}

__inline string SubStr(const char *str, const char *start, const char *end, bool inclusive)
{
	//buff[0] = '\0';
	string ret;
	const char *s = strstr(str, start);
	if(s)
	{
		if(!inclusive)
			s += strlen(start);
		const char *e;
		e = strstr(s, end);
		if(e)
		{
			if(inclusive)
				e += strlen(end);
			//unsigned int slen = min(len-1, (unsigned int)(e-s));
			//memcpy(buff, s, slen);
			//buff[slen] = '\0';
			ret = string(s, (unsigned int)(e-s));
		}
	}
	return ret;
}

bool SplitBy(const char *a, char c, vector<string> &strings)
{
	if(!a)
		return false;

	const char *s = a;
	const char *e = s;

	while(e)
	{
		e=strchr(s, c);
		if(e)
		{
			strings.push_back( string(s, (unsigned int)(e-s)) );
			s=e+1;
		}
	}
	strings.push_back( string(s) );

	return true;
}

bool SplitBy(const char *a, const char *c, vector<string> &strings)
{
	if(!a)
		return false;

	const char *s = a;
	const char *e = s;
	uint len=(uint)strlen(c);

	while(e)
	{
		e=strstr(s, c);
		if(e)
		{
			strings.push_back( string(s, (unsigned int)(e-s)) );
			s=e+len;
		}
	}
	strings.push_back( string(s) );

	return true;
}

bool SplitBy(char *a, char c, vector<char*> &strings)
{
	if(!a)
		return false;

	char *s = a;
	char *e = s;

	while(e)
	{
		e=strchr(s, c);
		if(e)
		{
			strings.push_back( s );
			//*e='\0';
			s=e+1;
		}
	}
	strings.push_back( s );

	return true;
}

bool SplitByTrimmed(const char *a, char c, vector<string> &strings)
{
	if(!a)
		return false;

	const char *s = a;
	const char *e = s;

	while(e)
	{
		e=strchr(s, c);
		if(e)
		{
			strings.push_back( string(s, (unsigned int)(e-s)) );
			s=e+1;
		}
	}
	//strings.push_back( string(s) );

	return true;
}

#endif

__inline int ToInt(string &s)
{
	int i=0;
	sscanf(s.c_str(), "%d", &i);
	return i;
}

__inline float ToFloat(string &s)
{
	float f=0.0f;
	sscanf(s.c_str(), "%g", &f);
	return f;
}

__inline double ToDouble(string &s)
{
	double d=0.0;
	sscanf(s.c_str(), "%lf", &d);
	return d;
}

__inline int ToInt(const char *s)
{
	int i=0;
	if(s)
		sscanf(s, "%d", &i);
	return i;
}

__inline unsigned int ToUInt(const char *s)
{
	unsigned int i=0;
	if(s)
		sscanf(s, "%u", &i);
	return i;
}

__inline __int64 ToInt64(const char *s)
{
	__int64 i=0;
	if(s)
		sscanf(s, "%ld", &i);
	return i;
}

__inline unsigned __int64 ToUInt64(const char *s)
{
	unsigned __int64 i=0;
	if(s)
		sscanf(s, "%lu", &i);
	return i;
}

__inline float ToFloat(const char *s)
{
	float f=0.0f;
	if(s)
		sscanf(s, "%g", &f);
	return f;
}

__inline double ToDouble(const char *s)
{
	double d=0.0;
	if(s)
		sscanf(s, "%lf", &d);
	return d;
}

__inline int ToIntCommas(char *s)
{
	return 0;
}

__inline int ToIntCommas(string &s)
{
	return 0;
}

__inline string ToLower(const char *s)
{
	if(s==NULL)
		return string("");
	string ret=s;
	for(uint i=0;i<ret.length();i++)
		if(ret[i] >= 'A' && ret[i] <= 'Z')
			ret[i] = ret[i]-'A' + 'a';
	return ret;
}

__inline string ToLower(const string &s)
{
	string ret=s;
	for(uint i=0;i<ret.length();i++)
		if(ret[i] >= 'A' && ret[i] <= 'Z')
			ret[i] = ret[i]-'A' + 'a';
	return ret;
}

__inline bool MatchTypos(const char *a, const char *b)//returns 1 on match
{
	if((!a) || (!b))
		return 0;

	return TypoHash(a) == TypoHash(b);
}

__inline bool FindInt(const char *a, int &i)
{
	if(!a)
		return false;
	for(; *a && (*a<'0' || *a>'9');a++);

	if(!*a)
		return false;

	sscanf(a, "%d", &i);
	return true;
}

/*class Rcout
{
public:
	FILE *outfile;

	Rcout()
	{
		outfile=stdout;
	}

	Rcout(FILE *file)
	{
		outfile=file;
	}

	Rcout(const char *filename)
	{
		outfile = fopen(filename, "a");
	}

	template<class A>
	Rcout &operator<<(A a)
	{
		std::stringstream ss;
		ss<<(a);
		fwrite(ss.str().c_str(), 1, ss.str().length(), outfile);
		return (*this);
	}
};
//extern Rcout rcout;

class RCOUT
{
public:
	char b[1024*16];
	size_t l;
	FILE *file;

	RCOUT()
	{
		file=stdout;
		l=0;
	}

	RCOUT(FILE *f)
	{
		file=f;
		l=0;
	}

	void flush()
	{
		if(l==0)
			return;
		if(file) fwrite(b, 1, l, file);
		l=0;
	}

	~RCOUT()
	{
		flush();
	}

	void output_string(const char *a, size_t length)
	{
		if(length+l>1024*16) {
			flush();
			if(length>1024*16) {
				if(file) fwrite(a, 1, length, file);
			} else {
				memmove(b, a, length);
				l=length;
			}
		} else {
			memmove(b+l, a, length);
			l+=length;
		}
	}

	template<class A>
	RCOUT &operator<<(A a);
};*/

//#define rcout RCOUT()

#ifdef WIN32
#undef sscanf
#endif

string str_replace(const char *in, const char *find, const char *replace);

#ifdef RAYHEADERSMAIN
/*template<class A>
RCOUT & RCOUT::operator<<(A a)
{
	auto &s = boost::lexical_cast<string>(a);
	output_string(s.c_str(), s.length());
	return *this;
	//return *this << boost::lexical_cast<string>(a);
}

template<>
RCOUT & RCOUT::operator<< <const string &>(const string &a)
{
	output_string(a.c_str(), a.length());
	return (*this);
}

template<>
RCOUT & RCOUT::operator<< <const char*>(const char* a)
{
	output_string(a,strlen(a));
	return (*this);
}

template<>
RCOUT & RCOUT::operator<< <int>(int a)
{
	char buf[12];
	size_t i=11;
	int neg=a<0;
	if(a<0) {
		a*=-1;
	}
	do {
		buf[i] = '0'+(a%10);
		a/=10;
		i--;
	} while(a>0);
	if(neg) {
		buf[i]='-';
		i--;
	}
	output_string((const char*)(buf+i+1), 11-(i));
	return (*this);
}

template<>
RCOUT & RCOUT::operator<< <uint>(uint a)
{
	char buf[12];
	size_t i=11;
	do {
		buf[i] = '0'+(a%10);
		a/=10;
		i--;
	} while(a>0);
	output_string((const char*)(buf+i+1), 11-(i));
	return (*this);
}*/

vector<string> SuperSplit(const char *source, const char *format)
{
	const char *sourceend = source + strlen(source);
	vector<string> res;
	vector<string> formatting;
	SplitBy(format, '%', formatting);

	for(uint i=0;i<formatting.size()-1 && source<sourceend;i++)
	{
		string s;
		if(formatting[i+1].length()==0)
			s=source+formatting[i].length();
		else
			s = SubStr(source, formatting[i].c_str(), formatting[i+1].c_str(), 0);
		if(s.length()==0)
		{
			if(strstr(source, formatting[i].c_str())==NULL)
				break;

			/*s=source;
			res.push_back(s);
			//source=sourceend;
			//break;
			continue;*/
		}
		source = strstr(source, formatting[i].c_str()) + formatting[i].length() + s.length();
		res.push_back(s);
	}
	return res;
}

vector<string> SuperSplit_R(const char *source, const char *format, bool trim=true)
{
	const char *sourceend = source + strlen(source);
	vector<string> res;
	vector<string> formatting;
	SplitBy(format, '%', formatting);
	while(source<sourceend)
	{
		for(uint i=0;i<formatting.size()-1 && source<sourceend;i++)
		{
			string s;
			if(formatting[i+1].length()==0)
				s=source+formatting[i].length();
			else
				s = SubStr(source, formatting[i].c_str(), formatting[i+1].c_str(), 0);
			if(s.length()==0)
			{
				if(strstr(source, formatting[i].c_str())==NULL)
				{
					if (trim == false) res.push_back(string(source));
					source=sourceend;
					break;
				}
				/*s=source;
				res.push_back(s);
				source=sourceend;
				break;*/
			}
			if ( strstr(source+formatting[i].length(), formatting[i + 1].c_str()) ) {
				source = strstr(source, formatting[i].c_str()) + formatting[i].length() + s.length();
				res.push_back(s);
			} else {
				res.push_back(string(source));
				source = sourceend;
				break;
			}
		}
		source += formatting.back().length();
	}
	return res;
}

vector<string> SuperSplit_R2(const char *source, const char *format)
{
	const char *sourceend = source + strlen(source);
	vector<string> res;
	vector<string> formatting;
	uint it=0;
	if(format[0]=='%') {
		format++;
		SplitBy(format, '%', formatting);
		auto *s = strstr(source,formatting[0].c_str());
		if(s) {
			string str(source,s-source);
			res.push_back(str);
			source=s;//+formatting[0].length();
			//it++;
		}
	} else {
		SplitBy(format, '%', formatting);
	}
	if(formatting.back().length()==0) formatting.pop_back();

	for(;source<sourceend && source;it++)
	{
		uint i=it%formatting.size();
		uint i2=(it+1)%formatting.size();
		string s;
		if(formatting[i2].length()==0)
			s=source+formatting[i].length();
		else
			s = SubStr(source, formatting[i].c_str(), formatting[i2].c_str(), 0);
		if(s.length()==0)
		{
			if(strstr(source, formatting[i].c_str())==NULL)
				break;
			s=source+formatting[i].length();
		}
		source = strstr(source, formatting[i].c_str()) + formatting[i].length() + s.length();
		res.push_back(s);
	}
	return res;
}

string str_replace(const char *in, const char *find, const char *replace)
{
	string ret;
	vector<string> split;
	SplitBy(in, find, split);
	for(uint i=0;i<split.size()-1;i++)
	{
		ret += split[i];
		ret += replace;
	}
	if(split.size())
		ret += split.back();
	return ret;
}

string UnescapeHtml(string in)
{
	string out=in;
//need to do &amp; last!!!! duh!!!!!
	out=str_replace(out.c_str(), "&lt;", "<");
	out=str_replace(out.c_str(), "&gt;", ">");
	out=str_replace(out.c_str(), "&nbsp;", " ");
	out = str_replace(out.c_str(), "&amp;", "&");
	out = str_replace(out.c_str(), "&quot;", "\'");
	out = str_replace(out.c_str(), "&#039;", "\'");
	return out;
}

#endif
