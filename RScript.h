#include <math.h>
#include <rayrand.h>
#include <raycontainers.h>
#include <time.h>
#include <fstream>

using namespace std;

//const int MAX_OPERATION_LENGTH = 256;
const int MAX_LINE_LENGTH = 65536;
const int MAX_OPS_PER_LINE = 4;
const int WORD_SIZE = sizeof(void*);
char *Operations[] = { /*"=", "+", "-", "*", "/", "^", "==", "&", "while", "for", "if", "else", "else if", "stack", "(int)", "return",*/ "" };
void PrintToScreen(char *buffer);
void PrintToScreen(int buffer);
void PrintToScreen(float buffer);
char* Input();

int Is_Alphanumeric(char c)
{
	if( (c >= 65 && c <= 90) || ( c >= 97 && c <= 122) )
		return 1;
	if( c >= 48 && c <= 57 )
		return 2;
	return 0;
}

int Is_Whitespace(char c)
{
	if( (c >= 8 && c <= 13) || c == ' ')
		return 1;
	return 0;
}

class Line : public ContObj<Line>
{
public:
	int linenum;
	int filepos;
};

class InputStream
{
public:
	virtual char _get()=0;
	virtual void _unget()=0;
	virtual int tellg()=0;
	virtual bool good()=0;
	virtual void close()=0;
	virtual void _seekg(int tpos)=0;
	char lastchar;
	int pos;
	int line;
	LinkedList<Line> lines;
	Line	*CurrLine;

	virtual void seekg(int tpos)
	{
		if(tpos<pos)
		{
			for(lines.pTemp=CurrLine,CurrLine=NULL;lines.pTemp!=NULL;lines.pTemp=lines.pTemp->pPrev)
			{
				if(lines.pTemp->filepos < tpos)
				{
					CurrLine = lines.pTemp;
					line = CurrLine->linenum;
					break;
				}
			}
		}
		else if(tpos>pos)
		{
			for(lines.pTemp=CurrLine,CurrLine=NULL;lines.pTemp!=NULL;lines.pTemp=lines.pTemp->pNext)
			{
				if(lines.pTemp->filepos > tpos)
				{
					if(lines.pTemp->pPrev != NULL)
					{
						CurrLine = lines.pTemp->pPrev;
						line = CurrLine->linenum;
					}
					break;
				}
				else if(lines.pTemp->pNext == NULL)
				{
					CurrLine = lines.pTemp;
					line = CurrLine->linenum;
					break;
				}
			}
		}
		else//they are equal
			return;

		if(CurrLine==NULL)
		{
			line=-1;
			CurrLine=lines.pHead;
		}

		_seekg(tpos);
	}

	char get()
	{
		lastchar = _get();
		pos++;
		if(lastchar=='\n')
		{
			line++;
			CurrLine = lines.pTemp = new Line;
			lines.pTemp->filepos = pos;
			lines.pTemp->linenum = line;
			lines.Create(CurrLine);
		}
		return lastchar;
	}

	void unget()
	{
		_unget();
		pos--;
		if(lastchar == '\n')
		{
			line--;
			if(CurrLine!=NULL)
				CurrLine=CurrLine->pPrev;
		}
	}

	virtual ~InputStream()
	{
		//close();
	}
	InputStream()
	{
		lastchar='\0';
		pos=0;
		line=1;
		CurrLine = new Line;
		CurrLine->filepos = 0;
		CurrLine->linenum = 1;
		lines.Create(CurrLine);
	}
};

class FileInputStream : public InputStream
{
public:
	ifstream file;
	virtual char _get()
	{
		return file.get();
	}

	virtual void _unget()
	{
		file.unget();
	}

	virtual void _seekg(int tpos)
	{
		pos = tpos;
		file.seekg(tpos);
	}

	virtual int tellg()
	{
		pos = (int)file.tellg();
		return (int)file.tellg();
	}

	virtual bool good()
	{
		return file.good();
	}

	virtual void close()
	{
		file.close();
	}

	FileInputStream(){}
	FileInputStream(char *filename)
	{
		file.open(filename, ios::binary);
	}
	virtual ~FileInputStream()
	{close();}
};

class WinFileInputStream : public InputStream
{
public:
	HANDLE file;//FILE_CURRENT
	long Size;
	//long Pos;
	char c;
	LARGE_INTEGER LargeInt;
	virtual char _get()
	{
		//pos++;
		c = 'F';
		//PrintToScreen("\r\nReadfile == ");
		ReadFile(file,&c,1,0,0);
		/*if( (c >= 'a' && c <='z') || (c>='A' && c<='Z'))
		{
			char t[2];
			t[0] = c;
			t[1] = '\0';
		}*/
		//SetFilePointer(file,1,0,FILE_CURRENT);
		return c;
	}

	virtual void _unget()
	{
		//file.unget();
		//LargeInt.QuadPart = -1;
		//pos--;
		//pos = Pos;
		pos = SetFilePointer(file,-1,0,FILE_CURRENT);
	}

	virtual void _seekg(int tpos)
	{
		pos = tpos;
		//Pos = tpos;
		SetFilePointer(file,pos,0,0);
		//file.seekg(tpos);
	}

	virtual int tellg()
	{
		//pos = Pos;
		pos = SetFilePointer(file,0,0,FILE_CURRENT);
		return pos;
	}

	virtual bool good()
	{
		if(file == INVALID_HANDLE_VALUE)
			return 0;
		if(pos > Size)
			return 0;
		return 1;
	}

	virtual void close()
	{
		CloseHandle(file);
	}

	WinFileInputStream(){}
	WinFileInputStream(char *filename)
	{
		wchar_t fname[1024];
		//Pos = pos;
		int i;
		for(i=0;i<1024 && filename[i]!='\0';i++)
			fname[i] = filename[i];
		fname[i] = L'\0';

		//file.open(filename, ios::binary);
		file = CreateFile(fname, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

		if(i == 80)
		{
			CloseHandle(file);
			file = CreateFile(fname, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
		}

		Size = SetFilePointer(file,0,0,FILE_END);
		SetFilePointer(file,0,0,0);

		i = GetLastError();
		PrintToScreen(i);
		PrintToScreen("\r\n");

		//char temp[1024];
		//ReadFile(file, temp, 1024, 0, 0);
		//temp[1023] = '\0';
		//PrintToScreen(temp);
		//PrintToScreen("\r\n");

		i = GetLastError();

		PrintToScreen(i);
		PrintToScreen("\r\npos == ");

		pos = SetFilePointer(file,0,0,0);

		PrintToScreen(pos);
		PrintToScreen("\r\n");

		i = GetLastError();

		PrintToScreen(i);
		PrintToScreen("\r\n");

		PrintToScreen("Size == ");
		PrintToScreen(Size);
		PrintToScreen("\r\n");
	}
	virtual ~WinFileInputStream(){close();}
};

class BufferInputStream : public InputStream
{
public:
	char *buffer;
	int iSize;
	virtual char _get()
	{
		return buffer[pos];
	}

	virtual void _unget()
	{
	}

	virtual void _seekg(int tpos)
	{
		pos = tpos;
	}

	virtual int tellg()
	{
		return pos;
	}

	virtual bool good()
	{
		if(pos>=iSize)
			return 0;
		return 1;
	}

	virtual void close()
	{
	}

	virtual ~BufferInputStream()
	{
	}

	BufferInputStream()
	{
		buffer = NULL;
	}
	BufferInputStream(char *buff)
	{
		buffer = buff;
		iSize = strlen(buff);
	}
};

class Variable : public ContObj<Variable>
{
public:
	char *Data;
	Variable()
	{
		Data=0;
	}
	virtual ~Variable(){};
};
class Code;
class Script;
class Function;

class Code : public ContObj<Code>
{
public:
	Script *script;//used for getting the stack
	Function *parent;//the parent for this code other than the script, used for getting the args
	char *retval;//word size
	int linenum;

	virtual int Execute() {return 0;}
	virtual void Interpret(char *buffer) {return;}

	Code()
	{
		retval = NULL;
		parent = NULL;
		script = NULL;
		linenum=-1;
	}

	virtual ~Code()
	{
	}
};

class CodeBlock : public Code
{
public:
	LinkedList<Code> code;
	int CurrLine;

	CodeBlock()
	{
		CurrLine = -1;
	}

	virtual ~CodeBlock()
	{
		while(code.pHead!=NULL)
			delete code.pHead;
	}

	virtual int Execute()
	{
		Code *TempCode;
		for(TempCode=code.pHead;TempCode!=NULL;TempCode=TempCode->pNext)
		{
			CurrLine = TempCode->linenum;
			TempCode->parent = parent;
			switch(TempCode->Execute())
			{
			case -1:
				PrintToScreen("Error executing code while running CodeBlock(line: ");
				PrintToScreen(CurrLine);
				PrintToScreen(")!\r\n");
				return -1;
			case 1:
				return 1;
			}
		}
		return 0;
	}
	virtual void Interpret(char *buffer);
};

class Function : public Code
{
public:
	Code *code;//pointer to the code for this function, could technically be a CodeBlock, another function, a line of code, or even a script
	char *args;
	int CurrLine;
	//int	argbytes;

	Function()
	{
		CurrLine=-1;
		args = 0;
		code = 0;
		//argbytes = 0;
		name = new char[64];
	}

	virtual ~Function()
	{
		//delete[] name;
		delete code;
	}

	inline int ExecArgs()
 	{
		//char *Targs = args;
		Code *TCode;
		for(TCode = (Code*)args;TCode!=NULL;TCode=TCode->pNext)
		{
			TCode->parent = parent;
			if(TCode->Execute()<0)
			{
				PrintToScreen("Error executing code while running function ");
				PrintToScreen(name);
				PrintToScreen("'s arguments(");
				if(TCode->name!=NULL)
					PrintToScreen(TCode->name);
				PrintToScreen(") (line: ");
				PrintToScreen(CurrLine);
				PrintToScreen(")!\r\n");
				return -1;
			}
		}
		//args = Targs;
		return 0;
	}

	virtual int Execute()
	{
		if(code!=NULL)
		{
			code->parent = this;
			switch(code->Execute())
			{
			case -1:
				PrintToScreen("Error executing code while running function ");
				PrintToScreen(name);
				PrintToScreen(" (line: ");
				PrintToScreen(linenum);
				PrintToScreen(")!\r\n");
				return -1;
			case 0:
				return 0;
			case 1:
				return 0;
			default:
				PrintToScreen("Unknown return code(");
				PrintToScreen((int)((Code*)args)->retval);
				PrintToScreen(")while running function ");
				PrintToScreen(name);
				PrintToScreen(" (line: ");
				PrintToScreen(linenum);
				PrintToScreen(")!\r\n");
				return 0;

			}
		}
		return 0;
	}
	virtual void Interpret(char *buffer);
};

class OP : public ContObj<OP>
{
public:
	LinkedList<Code> vars;
	Function *TFunc;

	OP()
	{
		TFunc = NULL;
	}

	virtual ~OP()
	{
	}
};

class CodeLine : public Code
{
public:
	LinkedList<OP> ops;

	CodeLine()
	{
	}

	virtual ~CodeLine()
	{
		while(ops.pHead!=NULL)
		{
			delete ops.pHead;
		}
	}

	int FindOperations(char *buffer, int opnum, int filepos);
	int ParseFuncCall(char *buffer, int opnum, int filepos);
	virtual int Execute();
	virtual void Interpret(char *buffer);
};

class Script : public Code
{
public:
	InputStream *file;
	//ifstream file;
	Hash<Code> functions;
	Hash<Variable> variables;
	LinkedList<Code> arguments;
	Function *MainFunction;
	char *stack;
	int StackSize;
	int FloatToStringAcc;
	RayRand32 rrand;

	Script()
	{
		file = 0;
		StackSize=-1;
		stack=NULL;
		parent=0;
		FloatToStringAcc=5;
		rrand.SetStrongSeed(time(0));
	}

	virtual ~Script()
	{
		delete[] stack;
		delete file;
		variables.Clear();
		functions.Clear();
		//while(functions.pRoot!=NULL)
		//	delete functions.pTail;
	}

	virtual void RegisterStdFunctions();

	virtual int Execute()
	{
		//*(int*)stack = 1337;
		//cout << "stack[0] == "<<*(int*)stack<<"\r\n";
		MainFunction = (Function*)functions.Get("main");
		if(MainFunction != NULL)
		{
			arguments.pTemp = new Code;
			arguments.pTemp->retval = "This is being passed to the program!\r\n";
			arguments.Create(arguments.pTemp);
			MainFunction->args = (char*)arguments.pHead;

			MainFunction->parent = 0;
			PrintToScreen("\r\n");
			if(MainFunction->Execute()<0)
			{
				PrintToScreen("Error executing script!\r\n");
				return -1;
			}
			PrintToScreen("Executing complete!\r\n");
		}
		else
		{
			//error!
			PrintToScreen("Error executing code! No main function found!\r\n");
			return -1;
		}
		//cout << "stack[0] == "<<*(int*)stack<<"\r\n";
		return 0;
	}
	virtual void Interpret(char *buffer)
	{
		buffer = new char[MAX_LINE_LENGTH];

		int i=0;
		memset(buffer,0,MAX_LINE_LENGTH);
		Function *TempFunc;
		buffer[i]=' ';
		while(file->good() && Is_Whitespace(buffer[i]))
		{
			buffer[i] = file->get();
		}
		linenum = file->line;
		i++;
		while(file->good())
		{
			while(file->good() && i < 9)
			{
				buffer[i] = file->get();
				i++;
			}
			buffer[i] = '\0';
			if(!strcmp(buffer, "SetStack:"))
			{
				buffer[0] = file->get();
				for(i=1;buffer[i-1]!=';';i++)
					buffer[i] = file->get();
				buffer[i]='\0';

				StackSize = atoi(&buffer[0]);
				break;
			}
			i=0;
		}
		i=0;
		if(StackSize==-1)
		{
			PrintToScreen("Error Interpreting code! Could not find \"SetStack:NumBytes;\"!\r\n");
			delete[] stack;
			delete[] buffer;
			return;
		}
		delete[] stack;
		stack = new char[StackSize];
		memset(stack, 0, StackSize);

		PrintToScreen("StackSize == ");
		PrintToScreen(StackSize);
		PrintToScreen("\r\n");

		RegisterStdFunctions();

		while(file->good())
		{
			buffer[i] = file->get();
			if(buffer[i] == '{' || buffer[i] == '}' || buffer[i] == '(' || buffer[i] == ')')
			{
				buffer[i+1] = '\0';
				PrintToScreen("Error Interpreting code! Found extra ");
				buffer[i+1] = '\0';
				PrintToScreen(&buffer[i]);
				PrintToScreen(" (line: ");
				PrintToScreen(file->line);
				PrintToScreen(")!\r\n");
				return;
			}
			else if(i==0 && buffer[i] != 's')
				i=0;
			else if(i==1 && buffer[i] != 'u')
				i=0;
			else if(i==2 && buffer[i] != 'b')
				i=0;
			else if(i==2)
			{
				i=0;
				buffer[i]='\0';
				while( !Is_Alphanumeric(buffer[i]) )
					buffer[i] = file->get();
				while( Is_Alphanumeric(buffer[i]) )
				{
					i++;
					buffer[i] = file->get();
					if(i>64)
					{
						PrintToScreen("Error Interpreting code! Function name longer than 64 characters (");
						buffer[i+1]='\0';
						PrintToScreen(buffer);
						PrintToScreen(") (line: ");
						PrintToScreen(file->line);
						PrintToScreen(")!\r\n");
						break;
					}
				}
				file->unget();
				buffer[i] = '\0';

				TempFunc = (Function*)functions.Get(buffer);
				if(TempFunc == NULL)
				{
					TempFunc = new Function;
					strcpy_s(TempFunc->name, 64, buffer);
					functions.Create(TempFunc);
					TempFunc->script = this;
					TempFunc->Interpret(buffer);
				}
				else if(TempFunc->code == NULL)
				{
					PrintToScreen("Defining previously declared function ");
					PrintToScreen(TempFunc->name);
					PrintToScreen(" (line: ");
					PrintToScreen(file->line);
					PrintToScreen(")\r\n");
					TempFunc->Interpret(buffer);
				}
				else
				{
					PrintToScreen("Error Interpreting code! Function redefinition ");
					PrintToScreen(TempFunc->name);
					PrintToScreen(" (line: ");
					PrintToScreen(file->line);
					PrintToScreen(")\r\n");
					TempFunc->Interpret(buffer);
				}
				i=0;
			}
			else if(i>2)
				i=0;
			else
			{
				i++;
			}
		}
		delete[] buffer;
		return;
	}
};

void Function::Interpret(char *buffer)
{
	int i=0;
	buffer[i] = ' ';
	linenum = script->file->line;
	while( Is_Whitespace(buffer[i]) )
	{
		buffer[i] = script->file->get();
	}
	if( buffer[i] == '(')
	{
		/*while( Is_Alphanumeric(buffer[i]) != 2)
			buffer[i] = script->file->get();
		while( Is_Alphanumeric(buffer[i]) == 2)
		{
			i++;
			buffer[i] = script->file->get();
		}
		buffer[i] = '\0';
		script->file->unget();
		argbytes = atoi(buffer);*/

		i=0;
		buffer[i] = script->file->get();
		while( Is_Whitespace(buffer[i]) || buffer[i] == ')')
		{
			i++;
			buffer[i] = script->file->get();
		}

		script->file->unget();
		if( buffer[i] == ';')//function declaration
		{
			PrintToScreen("Declaring function ");
			PrintToScreen(name);
			PrintToScreen(" (line: ");
			PrintToScreen(linenum);
			PrintToScreen(")\r\n");
		}
		else
		{
			PrintToScreen("Defining function ");
			PrintToScreen(name);
			PrintToScreen(" (line: ");
			PrintToScreen(linenum);
			PrintToScreen(")\r\n");
			if( buffer[i] == '{' )
			{
				code = new CodeBlock;
				code->parent = this;
				code->script = script;
				code->Interpret(buffer);
			}
			else
			{
				code = new CodeLine;
				code->parent = this;
				code->script = script;
				code->Interpret(buffer);
			}
		}
	}
	else
	{
		//error!
		PrintToScreen("Error Interpreting code! Missing \'(\' in function ");
		PrintToScreen(name);
		PrintToScreen(" definition!");
		PrintToScreen(" (line: ");
		PrintToScreen(linenum);
		PrintToScreen(")\r\n");
	}
	return;
}

void CodeBlock::Interpret(char *buffer)
{
	int i=0;
	Code *TCode;
	buffer[i] = script->file->get();

	while( Is_Whitespace(buffer[i]))
		buffer[++i] = script->file->get();
	linenum = script->file->line;
	if(buffer[i]=='{')
	{
		while(script->file->good())
		{
			buffer[i] = script->file->get();
			if( Is_Whitespace(buffer[i]));
			else if(buffer[i] != '}')
			{
				script->file->unget();

				if(buffer[i] == '{')
				{
					TCode = new CodeBlock;
					TCode->parent = parent;
					TCode->script = script;
					code.Create(TCode);
					TCode->Interpret(buffer);
				}
				else
				{
					TCode = new CodeLine;
					TCode->parent = parent;
					TCode->script = script;
					code.Create(TCode);
					TCode->Interpret(buffer);
				}
			}
			else
			{
				break;
			}
		}
		if(!script->file->good())
		{
			for(TCode=parent;TCode!=NULL;TCode=TCode->parent)
			{
				if(TCode->name != NULL)
				{
					PrintToScreen("Error Interpreting code! File error in code block in ");
					PrintToScreen(TCode->name);
					PrintToScreen(" (line: ");
					PrintToScreen(script->file->line);
					PrintToScreen(")!\r\n");
					return;
				}
			}
			PrintToScreen("Error Interpreting code! File error in code block in unknown function");
			PrintToScreen(" (line: ");
			PrintToScreen(script->file->line);
			PrintToScreen(")!\r\n");
		}
	}
	else
	{
		for(TCode=parent;TCode!=NULL;TCode=TCode->parent)
		{
			if(TCode->name != NULL)
			{
				PrintToScreen("Error Interpreting code! Could not find \'{\' for code block in ");
				PrintToScreen(TCode->name);
				PrintToScreen(" (line: ");
				PrintToScreen(script->file->line);
				PrintToScreen(")!\r\n");
				return;
			}
		}
		PrintToScreen("Error Interpreting code! Could not find \'{\' for code block in unknown function");
		PrintToScreen(" (line: ");
		PrintToScreen(script->file->line);
		PrintToScreen(")!\r\n");
	}
}

int CodeLine::FindOperations(char *buffer, int opnum, int filepos)
{
	return ParseFuncCall(buffer, opnum, filepos);
}

int CodeLine::ParseFuncCall(char *buffer, int opnum, int filepos)
{
	//0 is a function
	int i=0;
	int a=0;
	int e=0;
	int wsp=0;//whitespace
	char c;
	Code *TCode;
	//Code *TCode2;
	OP *op;

	if( Is_Whitespace(buffer[0]))
		return -1;
	for(i=0;i<MAX_LINE_LENGTH;i++)
	{
		if(wsp==0 && Is_Whitespace(buffer[i]))
			wsp = i;
		if(i!=0 && buffer[i] == '(')//if i==0 and buffer[i] == '(' then it is a cast not a function
		{
			a=0;
			if(wsp==0)
				wsp=i;

			op = new OP;
			op->name = new char[wsp+1];
			memcpy(op->name, buffer, wsp);
			op->name[wsp]='\0';
			
			if( strcmp(op->name, ";")==0)
			{
				PrintToScreen("function name ;");
				PrintToScreen(" (line: ");
				script->file->seekg(filepos+i);
				PrintToScreen(script->file->line);
				PrintToScreen(")!\r\n");
			}
			ops.Create(op);

			i++;
			wsp = i;
			while(buffer[i]!=')')
			{
				while( Is_Whitespace(buffer[wsp]))
							wsp++;
				if(buffer[i] == '\"')
				{
					a++;
					wsp = i++;
					while(i < MAX_LINE_LENGTH && (buffer[i] != '\"' || buffer[i-1] == '\\'))
						i++;
					if(i>= MAX_LINE_LENGTH)
					{
						PrintToScreen("Error Interpreting code! Missing end \" in function ");
						PrintToScreen(this->parent->name);
						PrintToScreen(" (line: ");
						script->file->seekg(filepos+i);
						PrintToScreen(script->file->line);

						PrintToScreen(")!\r\n");
						return i;
					}
					buffer[i] = '\0';

					TCode = new Code;
					script->file->seekg(filepos+i);
					TCode->linenum = script->file->line;
					
					TCode->script = script;
					TCode->parent = parent;
					op->vars.Create(TCode);
					wsp++;
					TCode->retval = new char[i-wsp+1];
					
					for(int e=0;wsp<i+1;wsp++,e++)
					{
						TCode->retval[e] = buffer[wsp];
						if(buffer[wsp]=='\\')
						{
							wsp++;
							if(buffer[wsp] == '"')
								TCode->retval[e] = '"';
							else if(buffer[wsp] == 'n')
								TCode->retval[e] = '\n';
							else if(buffer[wsp] == '\\')
								TCode->retval[e] = '\\';
							else if(buffer[wsp] == 'r')
								TCode->retval[e] = '\r';
							else if(buffer[wsp] == '0')
								TCode->retval[e] = '\0';
							else if(buffer[wsp] == 't')
								TCode->retval[e] = '\t';
							else
								TCode->retval[e] = buffer[wsp];
						}
						else if(buffer[wsp] == '\0')
							break;
					}

					//cout << "Found string literal \""<<TCode->retval<<"\"\r\n";

					while(i < MAX_LINE_LENGTH && buffer[i] != ',' && buffer[i] != ')')
						i++;
					i--;
					wsp=i+1;
				}
				else if(buffer[i] == '\'')
				{
					TCode = new Code;
					script->file->seekg(filepos+i);
					TCode->linenum = script->file->line;
					
					TCode->script = script;
					TCode->parent = parent;
					op->vars.Create(TCode);

					i++;
					if(buffer[i]=='\\')
					{
						i++;
						if(buffer[i] == '"')
							TCode->retval = (char*)'"';
						else if(buffer[i] == 'n')
							TCode->retval = (char*)'\n';
						else if(buffer[i] == '\\')
							TCode->retval = (char*)'\\';
						else if(buffer[i] == 'r')
							TCode->retval = (char*)'\r';
						else if(buffer[i] == '0')
							TCode->retval = (char*)'\0';
						else if(buffer[i] == 't')
							TCode->retval = (char*)'\t';
						else
							TCode->retval = (char*)buffer[i];
					}
					else
						TCode->retval = (char*)buffer[i];
					i++;
					if(buffer[i]!='\'')
					{
						PrintToScreen("Error Interpreting code! Missing end \' in function ");
						PrintToScreen(this->parent->name);
						PrintToScreen(" (line: ");
						script->file->seekg(filepos+i);
						PrintToScreen(script->file->line);

						PrintToScreen(")!\r\n");
						return i;
					}
					while(i < MAX_LINE_LENGTH && buffer[i] != ',' && buffer[i] != ')')
						i++;
					i--;
					wsp=i+1;
				}
				else if(buffer[i] == '{')
				{
					a++;
					script->file->seekg(filepos+i);

					TCode = new CodeBlock;
					TCode->linenum = script->file->line;
					TCode->script = script;
					TCode->parent = parent;
					op->vars.Create(TCode);
					char *c = new char[MAX_LINE_LENGTH];
					TCode->Interpret(c);
					delete[] c;
					i = (script->file->tellg())-(filepos);

					while(i < MAX_LINE_LENGTH && buffer[i] != ',' && buffer[i] != ')' && buffer[i] != '\0' )
						i++;

					if(buffer[i]==')' || buffer[i] == '\0')
						break;

					wsp=i+1;

					while( Is_Whitespace(buffer[wsp]))
						wsp++;
				}
				else if(buffer[i] == '(')
				{
					a++;
					TCode = new CodeLine;
					script->file->seekg(filepos+i);
					TCode->linenum = script->file->line;
					
					TCode->script = script;
					TCode->parent = parent;
					op->vars.Create(TCode);
					i=((CodeLine*)TCode)->ParseFuncCall(&buffer[wsp],opnum,filepos+wsp)+wsp;

					while(i < MAX_LINE_LENGTH && buffer[i] != ',' && buffer[i] != ')' )
						i++;
					if(buffer[i]==')')
						break;
					wsp=i+1;
					while( Is_Whitespace(buffer[wsp]))
						wsp++;
				}
				else if(buffer[i] == ',' || buffer[i+1] == ')')
				{
					if(buffer[i+1] == ')')
						i++;
					a++;
					c = buffer[i];
					buffer[i] = '\0';
					while( Is_Whitespace(buffer[wsp]))
						wsp++;
					int nums=0;
					for(e=wsp;e<i;e++)
					{
						if( Is_Alphanumeric(buffer[e]))
							nums++;
						if(buffer[e]=='.' && nums)
						{
							TCode = new Code;
							script->file->seekg(filepos+i);
							TCode->linenum = script->file->line;

							TCode->script = script;
							TCode->parent = parent;
							op->vars.Create(TCode);

							(*(float*)(&TCode->retval)) = (float)atof(&buffer[wsp]);
							break;
						}
					}
					if(e==i && nums)
					{
						TCode = new Code;
						script->file->seekg(filepos+i);
						TCode->linenum = script->file->line;
						
						TCode->script = script;
						TCode->parent = parent;
						op->vars.Create(TCode);
						*((int*)&TCode->retval) = atoi(&buffer[wsp]);
					}
					buffer[i] = c;
					wsp = i+1;
					if(buffer[i] == ')')
					{
						break;
					}
					else
					{
						while(i < MAX_LINE_LENGTH && buffer[i] != ',' && buffer[i] != ')')
							i++;
						wsp = i+1;
						while( Is_Whitespace(buffer[wsp]))
							wsp++;
					}
				}
				i++;
			}
			i++;
 			return i;
		}
	}
	return 0;
}

void CodeLine::Interpret(char *buffer)
{
	int i=0;
	int curlys=0;
	int parens=0;
	int filepos;
	int filepos2;

	filepos = script->file->tellg();
	linenum = script->file->line;

	buffer[i]=' ';
	//while( Is_Whitespace(buffer[i]) )
	buffer[i] = script->file->get();

	if(buffer[i] == '{')
		curlys++;
	else if(buffer[i] == '(')
		parens++;
	else if(buffer[i] == '}')
	{
		curlys--;
		if(curlys<0)
		{
			PrintToScreen("Error Interpreting code! Found extra right curly brace in function ");
			PrintToScreen(this->parent->name);
			PrintToScreen(" (line: ");
			PrintToScreen(script->file->line);
			PrintToScreen(")!\r\n");
			return;
		}
	}
	else if(buffer[i] == ')')
	{
		parens--;
		if(parens<0)
		{
			PrintToScreen("Error Interpreting code! Found extra right parenthesis in function ");
			PrintToScreen(this->parent->name);
			PrintToScreen(" (line: ");
			PrintToScreen(script->file->line);
			PrintToScreen(")!\r\n");
			return;
		}
	}
	while(! (buffer[i]==';' && curlys==0 && parens==0) )
	{
		i++;
		if(i>MAX_LINE_LENGTH-2)
		{
			for(Code *TCode=parent;TCode!=NULL;TCode=TCode->parent)
			{
				if(TCode->name != NULL)
				{
					PrintToScreen("Error Interpreting code! Line longer than MAX_LINE_LENGTH in ");
					PrintToScreen(TCode->name);
					PrintToScreen(" (line: ");
					PrintToScreen(script->file->line);
					PrintToScreen(")!\r\n");
					return;
				}
			}

			PrintToScreen("Error Interpreting code! Line longer than 64KB in unknown function");
			PrintToScreen(" (line: ");
			PrintToScreen(script->file->line);
			PrintToScreen(")!\r\n");
			return;
		}
		buffer[i] = script->file->get();

		if(buffer[i] == '{')
			curlys++;
		else if(buffer[i] == '(')
			parens++;
		else if(buffer[i] == '}')
		{
			curlys--;
			if(curlys<0)
			{
				PrintToScreen("Error Interpreting code! Found extra right curly brace in function ");
				PrintToScreen(this->parent->name);
				PrintToScreen(" (line: ");
				PrintToScreen(script->file->line);
				PrintToScreen(")!\r\n");
				return;
			}
		}
		else if(buffer[i] == ')')
		{
			parens--;
			if(parens<0)
			{
				PrintToScreen("Error Interpreting code! Found extra right parenthesis in function ");
				PrintToScreen(this->parent->name);
				PrintToScreen(" (line: ");
				PrintToScreen(script->file->line);
				PrintToScreen(")!\r\n");
				return;
			}
		}
	}
	filepos2 = script->file->tellg();

	if(filepos+i+1 != filepos2)
	{
		int x = filepos+i+1;
		PrintToScreen("filepos != filepos2");
		PrintToScreen(" (line: ");
		PrintToScreen(script->file->line);
		PrintToScreen("!\r\n");
	}

	i++;
	buffer[i] = '\0';

	i=0;

	int opnum=0;
	while(buffer[i]!='\0')
	{
		while( Is_Whitespace(buffer[i]) )
			i++;
		for(;buffer[i]!='\0' && (!Is_Whitespace(buffer[i]));i++)
		{
			if(buffer[i] !=';')
			{
				i+=ParseFuncCall(&buffer[i], opnum, filepos+i);
				script->file->seekg(filepos2);
				return;
			}
		}
	}
	script->file->seekg(filepos2);
}


int CodeLine::Execute()
{
	OP *TempOp;
	char *Targs;
	//ops.linenum = linenum;
	parent->CurrLine = linenum;
	for(TempOp=ops.pHead;TempOp!=NULL;TempOp=TempOp->pNext)
	{
		Function *TCode = TempOp->TFunc;
		if(TCode == NULL)
			TempOp->TFunc = TCode = (Function*)script->functions.Get(TempOp->name);
		if(TCode != NULL)
		{
			Targs = TCode->args;
			memcpy(&TCode->args, &TempOp->vars.pHead, WORD_SIZE);
			TCode->parent = parent;
			switch(TCode->Execute())
			{
			case -1:
				return -1;
			case 1:
				return 1;
			}
			TCode->args = Targs;
		}
		else
		{
			PrintToScreen("Function ");
			PrintToScreen(TempOp->name);
			PrintToScreen(" not found");
			PrintToScreen(" (line: ");
			PrintToScreen(linenum);
			PrintToScreen(")!\r\n");
			return -1;
		}
		retval = TCode->retval;
	}
	return 0;
}

#include "RScriptStdFunctions.h"
