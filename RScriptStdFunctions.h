namespace SLStdFunctions{

	namespace Util{

		class Var : public Function
		{
		public:
			Var()
			{
				strcpy_s(name, 64, "Var");
			}
			virtual ~Var(){}

			virtual int Execute()
			{
				ExecArgs();
				Variable *tvar = script->variables.Get(((Code*)args)->retval);
				if(tvar == NULL)
				{
					tvar = new Variable;
					tvar->name = new char[strlen(((Code*)args)->retval)+1];
					strcpy_s(tvar->name, strlen(((Code*)args)->retval)+1, ((Code*)args)->retval);
					script->variables.Create(tvar);
				}

				retval = (char*)&tvar->Data;
				return 0;
			}
		};

		class DeleteVar : public Function
		{
		public:
			DeleteVar()
			{
				strcpy_s(name, 64, "DeleteVar");
			}
			virtual ~DeleteVar(){}

			virtual int Execute()
			{
				ExecArgs();
				Variable *tvar = script->variables.Get(((Code*)args)->retval);
				if(tvar == NULL)
				{
					retval = 0;
					return 0;
				}

				delete tvar;
				retval = (char*)1;
				return 0;
			}
		};

		class Input : public Function
		{
		public:
			Input()
			{
				strcpy_s(name, 64, "Input");
			}

			virtual int Execute()
			{
				
				retval = ::Input();
				return 0;
			}
		};

		class ExecArgs : public Function
		{
		public:
			ExecArgs()
			{
				strcpy_s(name, 64, "ExecArgs");
			}

			virtual int Execute()
			{
				return ((Function*)parent)->ExecArgs();
			}
		};

		class ExecArg : public Function
		{
		public:
			ExecArg()
			{
				strcpy_s(name, 64, "ExecArg");
			}

			virtual int Execute()
			{
				//char *Targs = args;
				Code *TCode;
				int a = (int)((Code*)args)->retval;
				RayCont<Code> *TList;

				TList = ((Code*)((Function*)parent)->args)->Cont;
				TCode = TList->Get(a);
				if(TCode == NULL)
				{
					PrintToScreen("Warning executing code! ExecArg() requesting argument out of bounds!\r\n");
					return 0;
				}

				TCode->parent = parent;
				if(TCode->Execute()<0)
				{
					PrintToScreen("Error executing code while running function ");
					PrintToScreen(name);
					PrintToScreen("'s arguments(");
					if(TCode->name!=NULL)
						PrintToScreen(TCode->name);
					PrintToScreen(")!\r\n");
					return -1;
				}

				//args = Targs;
				return 0;
			}
		};

		class Stack : public Function
		{
		public:
			Stack()
			{
				strcpy_s(name, 64, "Stack");
			}
			virtual ~Stack()
			{
			}

			virtual int Execute()
			{
				ExecArgs();
				int a = (int)((Code*)args)->retval;
				if(a >= script->StackSize)
				{
					PrintToScreen("Error executing code! Requesting stack out of bounds!\r\n");
					return -1;
				}
				//char *c = &script->stack[a];
				retval = &script->stack[a];
				return 0;
			}
		};

		class Argument : public Function
		{
		public:
			Argument()
			{
				strcpy_s(name, 64, "Arg");
			}
			virtual ~Argument()
			{
			}

			virtual int Execute()
			{
				ExecArgs();
				int a = (int)((Code*)args)->retval;
				Code *TCode = ((Code*)((Function*)((Code*)args)->parent)->args)->Cont->Get(a);
				if(TCode == NULL)
				{
					PrintToScreen("Warning executing code! Arg() requesting argument out of bounds!\r\n");
					return 0;
				}
				retval = (char*)&(TCode->retval);
				return 0;
			}
		};

		class ArgumentCount : public Function
		{
		public:
			ArgumentCount()
			{
				strcpy_s(name, 64, "ArgCount");
			}
			virtual ~ArgumentCount()
			{
			}

			virtual int Execute()
			{
				ExecArgs();

				retval = (char*)((Code*)((Function*)parent)->args)->Cont->Objects;
				return 0;
			}
		};

		class Asterisk : public Function
		{
		public:
			Asterisk()
			{
				strcpy_s(name, 64, "*");
			}
			virtual ~Asterisk()
			{
			}

			virtual int Execute()
			{
				ExecArgs();
				if(*(int*)args==0)
				{
					PrintToScreen("Error executing code! Dereferencing NULL in function ");
					PrintToScreen(name);
					PrintToScreen("!\r\n");
					return -1;
				}
				retval = *(char **)(((Code*)args)->retval);
				//memcpy(retval, *(void**)args, WORD_SIZE);
				return 0;
			}
		};

		class Ampersand : public Function
		{
		public:
			Ampersand()
			{
				strcpy_s(name, 64, "&");
			}
			virtual ~Ampersand()
			{
			}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*)&((Code*)args)->retval;
				return 0;
			}
		};

		class StackTrace : public Function
		{
		public:
			StackTrace()
			{
				strcpy_s(name, 64, "StackTrace");
			}
			virtual ~StackTrace(){}

			virtual int Execute()
			{
				Function *TCode;
				int i;
				int a = 10;
				if( args != NULL)
					a = (int)((Code*)args)->retval;

				PrintToScreen("\r\n---------------Stack Trace---------------\r\n");
				for(TCode = this;TCode!=NULL;TCode=TCode->parent,a--)
				{
					if(a<=1)
					{
						PrintToScreen(".................\r\n");
						TCode = (Function*)script->MainFunction;
					}
					if(TCode->name!=NULL)
					{
						PrintToScreen("--Function\t");
						PrintToScreen(TCode->name);
						i = strlen(TCode->name)/4;
						for(;i<3;i++)
							PrintToScreen("\t");
						PrintToScreen("(line:\t");
						PrintToScreen(TCode->CurrLine);
						PrintToScreen(")\r\n");
					}
					else if(TCode == (Code*)script)
					{
						PrintToScreen("--Script\t");
						PrintToScreen("\t\t(line:\t");
						PrintToScreen(TCode->CurrLine);
						PrintToScreen(")\r\n");
					}
					else
					{
						PrintToScreen("--Unnamed\tfunction");
						PrintToScreen("\t(line:\t");
						PrintToScreen(TCode->CurrLine);
						PrintToScreen(")\r\n");
					}
					if(TCode == TCode->parent)
					{
						PrintToScreen("Recursion Detected! Stack Trace Failed!\r\n");
						break;
					}
					if(a<=1)
					{
						break;
					}
				}
				PrintToScreen("---------------Stack Trace---------------\r\n");
				return 0;
			}
		};

		class LineNum : public Function
		{
		public:
			LineNum()
			{
				strcpy_s(name, 64, "LineNumber");
			}
			virtual ~LineNum(){}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*)parent->CurrLine;//(char*)((Code*)args)->linenum;
				return 0;
			}
		};

		class WordSize : public Function
		{
		public:
			WordSize()
			{
				strcpy_s(name, 64, "WordSize");
			}
			virtual ~WordSize(){}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*)WORD_SIZE;
				return 0;
			}
		};

		class Sleep : public Function
		{
		public:
			Sleep()
			{
				strcpy_s(name, 64, "Sleep");
			}
			virtual ~Sleep(){}

			virtual int Execute()
			{
				ExecArgs();
				::Sleep((int)((Code*)args)->retval);
				return 0;
			}
		};

		class Eval : public Function
		{
		public:
			Eval()
			{
				strcpy_s(name, 64, "Eval");
			}

			virtual int Execute()
			{
				CodeBlock *TCode = new CodeBlock;
				TCode->parent = this;
				TCode->script = script;
				ExecArgs();

				InputStream *tstream = script->file;
				script->file = new BufferInputStream(((Code*)args)->retval);

				char *c = new char[MAX_LINE_LENGTH];

				TCode->Interpret(c);
				delete[] c;
				delete script->file;
				script->file = tstream;

				if(TCode->Execute() == -1)
				{
					PrintToScreen("Error executing code while running function ");
					PrintToScreen(name);
					PrintToScreen(" (line: ");
					PrintToScreen(linenum);
					PrintToScreen(")!\r\n");
				}
				delete TCode;
				return 0;
			}
		};

		class New : public Function
		{
		public:
			New()
			{
				strcpy_s(name, 64, "New");
			}
			virtual ~New(){}

			virtual int Execute()
			{
				ExecArgs();
				retval = new char[(int)((Code*)args)->retval];
				retval[0] = '\0';
				return 0;
			}
		};

		class Delete : public Function
		{
		public:
			Delete()
			{
				strcpy_s(name, 64, "Delete");
			}
			virtual ~Delete(){}

			virtual int Execute()
			{
				ExecArgs();
				for(Code* TCode=((Code*)args);TCode!=NULL;TCode=TCode->pNext)
				{
					try{
						delete[] TCode->retval;
					}
					catch(...)
					{
						PrintToScreen("Delete Error!\r\n");
					}
					TCode->retval = NULL;
				}
				return 0;
			}
		};

		class Fail : public Function
		{
		public:
			Fail()
			{
				strcpy_s(name, 64, "Fail");
			}
			virtual ~Fail(){}

			virtual int Execute()
			{
				return -1;
			}
		};

		class Assign : public Function
		{
		public:
			Assign()
			{
				strcpy_s(name, 64, "=");
			}
			virtual ~Assign()
			{
			}

			virtual int Execute()
			{
				Code* TCode;
				ExecArgs();

				TCode = ((Code*)args)->pNext;
				if(TCode == NULL)
				{
					PrintToScreen("Error executing code! Missing an argument for Assign function!\r\n");
					return -1;
				}

				*(char**)((Code*)args)->retval = TCode->retval;			
				return 0;
			}
		};

		class Time : public Function
		{
		public:
			Time()
			{
				strcpy_s(name, 64, "Time");
			}
			virtual ~Time()
			{
			}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*)time(0);	
				return 0;
			}
		};

		class Rand : public Function
		{
		public:
			Rand()
			{
				strcpy_s(name, 64, "Rand");
			}
			virtual ~Rand(){}

			virtual int Execute()
			{
				ExecArgs();
				int a = script->rrand.iRand32();
				if(a<0)
					a*=-1;
				if( args != NULL )
				{
					int b = (int)((Code*)args)->retval;
					if( ((Code*)args)->pNext != NULL)
					{
						a %= ((int)((Code*)args)->pNext->retval) - b;
						a += b;

					}
					else
					{
						a %= b;
					}
				}
				retval = (char*)a;
				return 0;
			}
		};

		class SRand : public Function
		{
		public:
			SRand()
			{
				strcpy_s(name, 64, "SRand");
			}
			virtual ~SRand(){}

			virtual int Execute()
			{
				ExecArgs();
				script->rrand.SetStrongSeed( (unsigned int)((Code*)args)->retval );
				retval = (char*)script->rrand.newseed;
				return 0;
			}
		};

		class System : public Function
		{
		public:
			System()
			{
				strcpy_s(name, 64, "System");
			}
			virtual ~System(){}

			virtual int Execute()
			{
				ExecArgs();
				#ifndef PHONE
				retval = (char*)system(((Code*)args)->retval);
				#endif
				return 0;
			}
		};
	};



	namespace iMath {

		class PlusEquals : public Function
		{
		public:
			PlusEquals()
			{
				strcpy_s(name, 64, "+=");
			}

			virtual int Execute()
			{
				int *a;
				Code *TCode;
				ExecArgs();

				a = (int*)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					*a += (int)TCode->retval;

				*(int*)&retval = *a;
				return 0;
			}
		};

		class Add : public Function
		{
		public:
			Add()
			{
				strcpy_s(name, 64, "+");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a += (int)TCode->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Subtract : public Function
		{
		public:
			Subtract()
			{
				strcpy_s(name, 64, "-");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a -= (int)TCode->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Multiply : public Function
		{
		public:
			Multiply()
			{
				strcpy_s(name, 64, "X");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a *= (int)TCode->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Divide : public Function
		{
		public:
			Divide()
			{
				strcpy_s(name, 64, "/");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a /= (int)TCode->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Mod : public Function
		{
		public:
			Mod()
			{
				strcpy_s(name, 64, "%");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a %= (int)TCode->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Exponent : public Function
		{
		public:
			Exponent()
			{
				strcpy_s(name, 64, "^");
			}

			virtual int Execute()
			{
				int a,b;
				int i;
				Code *TCode;
				ExecArgs();

				a = b = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
				{
					if((int)TCode->retval == 0)
						a = 1;
					else
					{
						for(i=1;i< (int)TCode->retval;i++)
							a *= b;
					}
					b=a;
				}

				*(int*)&retval = a;
				return 0;
			}
		};

		class Average : public Function
		{
		public:
			Average()
			{
				strcpy_s(name, 64, "Avg");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a += (int)TCode->retval;

				a /= ((Code*)args)->Cont->Objects;

				*(int*)&retval = a;
				return 0;
			}
		};

		class Min : public Function
		{
		public:
			Min()
			{
				strcpy_s(name, 64, "Min");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
				{
					if((int)TCode->retval < a)
						a = (int)TCode->retval;
				}

				*(int*)&retval = a;
				return 0;
			}
		};

		class Max : public Function
		{
		public:
			Max()
			{
				strcpy_s(name, 64, "Max");
			}

			virtual int Execute()
			{
				int a;
				Code *TCode;
				ExecArgs();

				a = (int)((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
				{
					if((int)TCode->retval > a)
						a = (int)TCode->retval;
				}

				*(int*)&retval = a;
				return 0;
			}
		};

		class LessThan : public Function
		{
		public:
			LessThan()
			{
				strcpy_s(name, 64, "<");
			}

			virtual int Execute()
			{
				int i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = (int)TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i < (int)TCode->retval)
						i = (int)TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class GreaterThan : public Function
		{
		public:
			GreaterThan()
			{
				strcpy_s(name, 64, ">");
			}

			virtual int Execute()
			{
				int i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = (int)TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i > (int)TCode->retval)
						i = (int)TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class LessThanEqual : public Function
		{
		public:
			LessThanEqual()
			{
				strcpy_s(name, 64, "<=");
			}

			virtual int Execute()
			{
				int i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = (int)TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i <= (int)TCode->retval)
						i = (int)TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class GreaterThanEqual : public Function
		{
		public:
			GreaterThanEqual()
			{
				strcpy_s(name, 64, ">=");
			}

			virtual int Execute()
			{
				int i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = (int)TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i >= (int)TCode->retval)
						i = (int)TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class EqualTo : public Function
		{
		public:
			EqualTo()
			{
				strcpy_s(name, 64, "==");
			}

			virtual int Execute()
			{
				int i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = (int)TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i == (int)TCode->retval)
						i = (int)TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};
	};

	namespace fMath {
		class Add : public Function
		{
		public:
			Add()
			{
				strcpy_s(name, 64, "f+");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&(((Code*)args)->retval);

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a += *(float*)&(TCode->retval);

				*(float*)&retval = a;
				return 0;
			}
		};

		class Subtract : public Function
		{
		public:
			Subtract()
			{
				strcpy_s(name, 64, "f-");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&(((Code*)args)->retval);

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a -= *(float*)&TCode->retval;

				*(float*)&retval = a;
				return 0;
			}
		};

		class Multiply : public Function
		{
		public:
			Multiply()
			{
				strcpy_s(name, 64, "fX");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a *= *(float*)&TCode->retval;

				*(float*)&retval = a;
				return 0;
			}
		};

		class Divide : public Function
		{
		public:
			Divide()
			{
				strcpy_s(name, 64, "f/");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a /= *(float*)&TCode->retval;

				*(float*)&retval = a;
				return 0;
			}
		};

		class Mod : public Function
		{
		public:
			Mod()
			{
				strcpy_s(name, 64, "f%");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a = fmod(a, *(float*)&TCode->retval);

				*(float*)&retval = a;
				return 0;
			}
		};

		class Exponent : public Function
		{
		public:
			Exponent()
			{
				strcpy_s(name, 64, "f^");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a = powf(a, *(float*)&TCode->retval);

				*(float*)&retval = a;
				return 0;
			}
		};

		class Average : public Function
		{
		public:
			Average()
			{
				strcpy_s(name, 64, "fAvg");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
					a += *(float*)&TCode->retval;

				a /= *(float*)&((Code*)args)->Cont->Objects;

				*(float*)&retval = a;
				return 0;
			}
		};

		class Min : public Function
		{
		public:
			Min()
			{
				strcpy_s(name, 64, "fMin");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
				{
					if(*(float*)&TCode->retval < a)
						a = *(float*)&TCode->retval;
				}

				*(float*)&retval = a;
				return 0;
			}
		};

		class Max : public Function
		{
		public:
			Max()
			{
				strcpy_s(name, 64, "fMax");
			}

			virtual int Execute()
			{
				float a;
				Code *TCode;
				ExecArgs();

				a = *(float*)&((Code*)args)->retval;

				TCode = ((Code*)args)->pNext;
				for(;TCode!=NULL;TCode=TCode->pNext)
				{
					if(*(float*)&TCode->retval > a)
						a = *(float*)&TCode->retval;
				}

				*(float*)&retval = a;
				return 0;
			}
		};

		class LessThan : public Function
		{
		public:
			LessThan()
			{
				strcpy_s(name, 64, "f<");
			}

			virtual int Execute()
			{
				float i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = *(float*)&TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i < *(float*)&TCode->retval)
						i = *(float*)&TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class GreaterThan : public Function
		{
		public:
			GreaterThan()
			{
				strcpy_s(name, 64, "f>");
			}

			virtual int Execute()
			{
				float i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = *(float*)&TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i > *(float*)&TCode->retval)
						i = *(float*)&TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class LessThanEqual : public Function
		{
		public:
			LessThanEqual()
			{
				strcpy_s(name, 64, "f<=");
			}

			virtual int Execute()
			{
				float i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = *(float*)&TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i <= *(float*)&TCode->retval)
						i = *(float*)&TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class GreaterThanEqual : public Function
		{
		public:
			GreaterThanEqual()
			{
				strcpy_s(name, 64, "f>=");
			}

			virtual int Execute()
			{
				float i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = *(float*)&TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i >= *(float*)&TCode->retval)
						i = *(float*)&TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};

		class EqualTo : public Function
		{
		public:
			EqualTo()
			{
				strcpy_s(name, 64, "f==");
			}

			virtual int Execute()
			{
				float i;
				Code *TCode;
				ExecArgs();
				TCode = ((Code*)args);
				i = *(float*)&TCode->retval;
				TCode=TCode->pNext;
				for(; TCode!=NULL; TCode=TCode->pNext)
				{
					if(i == *(float*)&TCode->retval)
						i = *(float*)&TCode->retval;
					else
					{
						retval = 0;
						return 0;
					}
				}
				retval = (char*)1;
				return 0;
			}
		};
	};

	namespace Math
	{
		class IntToFloat : public Function
		{
		public:
			IntToFloat()
			{
				strcpy_s(name, 64, "IntToFloat");
			}
			virtual ~IntToFloat()
			{
			}

			virtual int Execute()
			{
				float a;
				ExecArgs();

				a = (float)(int)((Code*)args)->retval;

				*(float*)&retval = a;
				return 0;
			}
		};

		class FloatToInt : public Function
		{
		public:
			FloatToInt()
			{
				strcpy_s(name, 64, "FloatToInt");
			}
			virtual ~FloatToInt()
			{
			}

			virtual int Execute()
			{
				int a;
				ExecArgs();

				a = (int)*(float*)&((Code*)args)->retval;

				*(int*)&retval = a;
				return 0;
			}
		};

	};
	namespace Printing {
		class Print : public Function
		{
		public:
			Print()
			{
				strcpy_s(name, 64, "Print");
			}
			virtual ~Print(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode = (Code*)args;

				PrintToScreen(TCode->retval);
				TCode = TCode->pNext;
				if(TCode!=NULL)
				{
					//PrintToScreen("\r\n");
					for(;TCode!=NULL;TCode=TCode->pNext)
					{
						PrintToScreen(TCode->retval);
						//PrintToScreen("\r\n");
					}
				}
				return 0;
			}
		};

		class PrintInt : public Function
		{
		public:
			PrintInt()
			{
				strcpy_s(name, 64, "PrintInt");
			}
			virtual ~PrintInt(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode = (Code*)args;

				PrintToScreen((int)TCode->retval);
				TCode = TCode->pNext;
				if(TCode!=NULL)
				{
					PrintToScreen("\r\n");
					for(;TCode!=NULL;TCode=TCode->pNext)
					{
						PrintToScreen((int)TCode->retval);
						PrintToScreen("\r\n");
					}
				}
				return 0;
			}
		};

		class PrintFloat : public Function
		{
		public:
			PrintFloat()
			{
				strcpy_s(name, 64, "PrintFloat");
			}
			virtual ~PrintFloat(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode = (Code*)args;

				PrintToScreen(*(float*)&TCode->retval);
				TCode = TCode->pNext;
				if(TCode!=NULL)
				{
					PrintToScreen("\r\n");
					for(;TCode!=NULL;TCode=TCode->pNext)
					{
						PrintToScreen(*(float*)&TCode->retval);
						PrintToScreen("\r\n");
					}
				}
				return 0;
			}
		};

		class PrintMix : public Function
		{
		public:
			PrintMix()
			{
				strcpy_s(name, 64, "PrintMix");
			}
			virtual ~PrintMix(){}

			virtual int Execute()
			{
				ExecArgs();
				for(Code *TCode = (Code*)args; TCode!=NULL; TCode=TCode->pNext)
				{
					switch( (int)TCode->retval)
					{
					case 0:
						TCode = TCode->pNext;
						if(TCode==NULL)
							return 0;
						PrintToScreen(TCode->retval);
						break;
					case 1:
						TCode = TCode->pNext;
						if(TCode==NULL)
							return 0;
						PrintToScreen((int)TCode->retval);
						break;
					case 2:
						TCode = TCode->pNext;
						if(TCode==NULL)
							return 0;
						PrintToScreen(*(float*)&TCode->retval);
						break;
					}
				}
				return 0;
			}
		};

		class PrintFunctions : public Function
		{
		public:
			PrintFunctions()
			{
				strcpy_s(name, 64, "PrintFunctions");
			}
			virtual ~PrintFunctions(){}

			virtual int Execute()
			{
				ExecArgs();
				int i,a;
				Function *TFunc;
				PrintToScreen("\r\n---------------Functions---------------\r\n");
				PrintToScreen("There are ");
				PrintToScreen(script->functions.Objects);
				PrintToScreen(" functions.\r\n");
				for(i=0;i<script->functions.Objects;i++)
				{
					PrintToScreen("---");
					a=i;
					TFunc = (Function*)script->functions.OrderedGet(a);
					if(TFunc==0)
					{
						PrintToScreen("NULL Function!\r\n");
						continue;
					}
					if(TFunc->name!=NULL)
					{
						PrintToScreen(TFunc->name);PrintToScreen("\r\n");
					}
					else
						PrintToScreen("Unnamed function!\r\n");
				}
				PrintToScreen("---------------Functions---------------\r\n");
				return 0;
			}
		};

		class PrintVariables : public Function
		{
		public:
			PrintVariables()
			{
				strcpy_s(name, 64, "PrintVariables");
			}
			virtual ~PrintVariables(){}

			virtual int Execute()
			{
				ExecArgs();
				int i,a;
				char buffer[48];
				//char c;
				Variable *TVar;
				PrintToScreen("\r\n---------------Variables---------------\r\n");
				PrintToScreen("There are ");
				PrintToScreen(script->variables.Objects);
				PrintToScreen(" variables.\r\n");
				for(i=0;i<script->variables.Objects;i++)
				{
					PrintToScreen("---");
					a=i;
					TVar = (Variable*)script->variables.OrderedGet(a);
					if(TVar==0)
					{
						PrintToScreen("NULL Variable!\r\n");
						continue;
					}
					if(TVar->name!=NULL)
					{
						PrintToScreen("\"");
						PrintToScreen(TVar->name);
						PrintToScreen("\"");
						a = strlen(TVar->name)/5;
						for(;a<3;a++)
							PrintToScreen("\t");
						PrintToScreen("\t");
					}
					else
						PrintToScreen("Unnamed variable!\t\t");
					PrintToScreen((int)TVar->Data);
					if((int)TVar->Data < 10000000)
						PrintToScreen("\t");
					PrintToScreen("\t(");

					if(WORD_SIZE==8)
					{
						memset(buffer, '0', 32);
						_itoa_s( ((int*)&TVar->Data)[1], buffer, 32, 16);
						_itoa_s( ((int*)&TVar->Data)[0], &buffer[8], 30, 16);
						if(strlen(buffer) < 8)
							buffer[strlen(buffer)] = '0';
					}
					else
						_itoa_s( ((int*)&TVar->Data)[0], buffer, 32, 16);

					PrintToScreen("0x");
					a=(WORD_SIZE*2) - strlen(buffer);
					for(;a>0;a--)
						PrintToScreen("0");
					PrintToScreen(buffer);

					PrintToScreen(")\r\n");
				}
				PrintToScreen("---------------Variables---------------\r\n");
				return 0;
			}
		};
	};

	namespace ControlStatements {
		class If : public Function
		{
		public:
			If()
			{
				strcpy_s(name, 64, "if");
			}
			virtual ~If()
			{
			}

			virtual int Execute()
			{
				Code *Targs = ((Code*)args);
				for(;Targs!=NULL;Targs=Targs->pNext)
				{
					Targs->Execute();
					if(Targs->pNext == NULL)
					{
						retval = 0;
						return 0;
					}
					else if((int)Targs->retval)
					{
						Targs = Targs->pNext;
						Targs->Execute();
						retval = (char*)1;
						return 0;
					}
					else
					{
						Targs = Targs->pNext;
					}
				}
				retval = 0;
				return 0;
			}
		};

		class While : public Function
		{
		public:
			While()
			{
				strcpy_s(name, 64, "while");
			}
			virtual ~While()
			{
			}

			virtual int Execute()
			{
				char *Targs = args;
				((Code*)Targs)->Execute();
				while((int)((Code*)Targs)->retval)
				{
					((Code*)Targs)->pNext->Execute();
					((Code*)Targs)->Execute();
				}
				return 0;
			}
		};

		class For : public Function
		{
		public:
			For()
			{
				strcpy_s(name, 64, "for");
			}
			virtual ~For()
			{
			}

			virtual int Execute()
			{
				//char *Targs = args;
				//RayCont<Code> *TLL = ((Code*)Targs)->Cont;

				Code *TC0 = ((Code*)args);
				Code *TC1 = TC0->pNext;
				Code *TC2 = TC1->pNext;
				Code *TC3 = TC2->pNext;

				for(TC0->Execute(); TC1->Execute()==0 && (int)TC1->retval;TC2->Execute())
					TC3->Execute();
				return 0;
			}
		};

		class Return : public Function
		{
		public:
			Return()
			{
				strcpy_s(name, 64, "return");
			}
			virtual ~Return(){}

			virtual int Execute()
			{
				ExecArgs();
				retval = ((Code*)args)->parent->retval = ((Code*)args)->retval;
				return 1;
			}
		};
	};
	namespace Strings
	{
		class Concat : public Function
		{
		public:
			Concat()
			{
				strcpy_s(name, 64, "StrConcat");
			}
			virtual ~Concat(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode;
				for(TCode = ((Code*)args)->pNext;TCode!=NULL;TCode=TCode->pNext)
				{
					strcat( ((Code*)args)->retval, TCode->retval);
				}
				retval = ((Code*)args)->retval;
				return 0;
			}
		};

		class ConcatInt : public Function
		{
		public:
			ConcatInt()
			{
				strcpy_s(name, 64, "StrConcatInt");
			}
			virtual ~ConcatInt(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode;
				char *buffer;
				for(buffer=((Code*)args)->retval;buffer[0]!='\0';buffer++);
				for(TCode = ((Code*)args)->pNext;TCode!=NULL;TCode=TCode->pNext)
				{
					_itoa_s( (int)TCode->retval, buffer, 64,10);
				}
				retval = ((Code*)args)->retval;
				return 0;
			}
		};

		class ConcatFloat : public Function
		{
		public:
			ConcatFloat()
			{
				strcpy_s(name, 64, "StrConcatFloat");
			}
			virtual ~ConcatFloat(){}

			virtual int Execute()
			{
				ExecArgs();
				Code *TCode;
				char *buffer;
				int i;
				for(buffer=((Code*)args)->retval;buffer[0]!='\0';buffer++);

				for(TCode = ((Code*)args)->pNext;TCode!=NULL;TCode=TCode->pNext)
				{
					i = (int)*(float*)&TCode->retval;
					_itoa_s( i, buffer, 64,10);
					i = strlen(buffer) + script->FloatToStringAcc;
					#ifdef PHONE
					sprintf(buffer, "%f", *(float*)&TCode->retval);
					#else
					_gcvt_s(buffer, 64, *(float*)&TCode->retval, i);
					#endif
					//strcat( ((Code*)args)->retval, buffer);
				}

				for(;buffer[0]!='\0';buffer++);
				buffer--;
				if(buffer[0] == '.')
					buffer[0] = '\0';

				retval = ((Code*)args)->retval;
				return 0;
			}
		};

		class SetFloatStringAcc : public Function
		{
		public:
			SetFloatStringAcc()
			{
				strcpy_s(name, 64, "StrSetFloatAcc");
			}
			virtual ~SetFloatStringAcc(){}

			virtual int Execute()
			{
				ExecArgs();
				script->FloatToStringAcc = (int)((Code*)args)->retval;
				retval = ((Code*)args)->retval;
				return 0;
			}
		};

		class SetChar : public Function
		{
		public:
			SetChar()
			{
				strcpy_s(name, 64, "StrSetChar");
			}
			virtual ~SetChar(){}

			virtual int Execute()
			{
				ExecArgs();
				int i;
				Code *TCode = ((Code*)args)->pNext;
				if(TCode->pNext==NULL)
				{
					i=0;
				}
				else
				{
					i = (int)TCode->retval;
					TCode = TCode->pNext;
				}
				((Code*)args)->retval[i] = (char)TCode->retval;
				retval = ((Code*)args)->retval;
				return 0;
			}
		};

		class StringToInt : public Function
		{
		public:
			StringToInt()
			{
				strcpy_s(name, 64, "StringToInt");
			}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*)atoi( ((Code*)args)->retval );
				return 0;
			}
		};

		class StringToFloat : public Function
		{
		public:
			StringToFloat()
			{
				strcpy_s(name, 64, "StringToFloat");
			}

			virtual int Execute()
			{
				ExecArgs();
				float f;
				f = atof( ((Code*)args)->retval );
				retval = (char*)*(int*)&f;
				return 0;
			}
		};

		class StrFind : public Function
		{
		public:
			StrFind()
			{
				strcpy_s(name, 64, "StrFind");
			}

			virtual int Execute()
			{
				ExecArgs();
				int a,i;
				char *str1,*str2;
				str1 = ((Code*)args)->retval;
				str2 = ((Code*)args)->pNext->retval;
				for(;str1[0]!='\0';str1++)
				{
					for(i=0,a=0;str2[i] == str1[a] || str2[i] == '\0';i++,a++)
					{
						if(str2[i] == '\0')
						{
							retval = (char*)(str1 - ((Code*)args)->retval);
							return 0;
						}
					}
				}
				retval = (char*)-1;
				return 0;
			}
		};

		class Strlen : public Function
		{
		public:
			Strlen()
			{
				strcpy_s(name, 64, "StrLen");
			}

			virtual int Execute()
			{
				ExecArgs();
				retval = (char*) strlen( ((Code*)args)->retval );
				return 0;
			}
		};

		class StrCmp : public Function
		{
		public:
			StrCmp()
			{
				strcpy_s(name, 64, "StrCmp");
			}

			virtual int Execute()
			{
				ExecArgs();
				if(args != NULL && ((Code*)args)->pNext != NULL)
					retval = (char*) strcmp( ((Code*)args)->retval, ((Code*)args)->pNext->retval );
				else
					retval = 0;
				return 0;
			}
		};

		class Strcpy : public Function
		{
		public:
			Strcpy()
			{
				strcpy_s(name, 64, "StrCpy");
			}

			virtual int Execute()
			{
				ExecArgs();
				int i,num=0;
				char *src,*dst;
				dst = ((Code*)args)->retval;
				src = ((Code*)args)->pNext->retval;
				if(((Code*)args)->pNext->pNext!=NULL)
					num  = (int)((Code*)args)->pNext->pNext->retval;
				if(num==0 || num > strlen(src))
					num = strlen(src);
				for(i=0;i< num; i++)
				{
					dst[i] = src[i];
				}
				dst[i] = '\0';
				retval = dst;
				return 0;
			}
		};

		class Replace : public Function
		{
		public:
			Replace()
			{
				strcpy_s(name, 64, "StrReplace");
			}
			virtual ~Replace(){}

			virtual int Execute()
			{
				ExecArgs();
				char *Str = ((Code*)args)->retval;
				int istrlen;
				char *TempStr;
				//memcpy(TempStr, Str, istrlen);
				Code *TCode;
				Code *TCode2;
				//TCode = ((Code*)args)->pNext;
				//TCode2 = TCode->pNext;
				int a,e,i;
				for(TCode = ((Code*)args)->pNext, TCode2 = TCode->pNext; TCode != NULL && TCode2 != NULL; TCode=TCode2->pNext)
				{
					if(TCode==NULL)
						break;
					else
						TCode2=TCode->pNext;

					istrlen = strlen(Str)+1;
					TempStr = new char[istrlen];
					memcpy(TempStr, Str, istrlen);

					for(a=0,e=0;TempStr[a]!='\0';a++,e++)
					{
						Str[e] = TempStr[a];
						for(i=0;TCode->retval[i] == TempStr[a+i] && TempStr[a+i] != '\0';i++);
						if(TCode->retval[i] == '\0')
						{
							a+=i-1;
							for(i=0;TCode2->retval[i]!='\0';i++,e++)
							{
								Str[e] = TCode2->retval[i];
							}
							e--;
						}
					}
					Str[e] = '\0';
					delete[] TempStr;
				}

				retval = Str;
				return 0;
			}
		};
	};
	namespace Windows
	{
		class SendKeyboardKey : public Function
		{
		public:
			SendKeyboardKey()
			{
				strcpy_s(name, 64, "SendKeyboardKey");
			}

			virtual int Execute()
			{
				ExecArgs();
				
				keybd_event((int)((Code*)args)->retval, 0, 0, 0);
				Sleep(1);
				keybd_event((int)((Code*)args)->retval, 0, KEYEVENTF_KEYUP, 0);
				retval = 0;

				return 0;
			}
		};
	};
};


void Script::RegisterStdFunctions()
{
	Code *TCode;

	//Util--------------------------------
	TCode = new SLStdFunctions::Util::Ampersand;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Argument;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::ArgumentCount;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Assign;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Asterisk;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Delete;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Eval;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::ExecArg;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::ExecArgs;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Fail;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::LineNum;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::New;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Sleep;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Stack;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::StackTrace;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Var;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::DeleteVar;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::WordSize;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Rand;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::SRand;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Time;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::System;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Util::Input;
	TCode->script = this;
	functions.Create(TCode);

	//Math--------------------------------
	TCode = new SLStdFunctions::Math::FloatToInt;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Math::IntToFloat;
	TCode->script = this;
	functions.Create(TCode);

	//iMath--------------------------------
	TCode = new SLStdFunctions::iMath::PlusEquals;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Add;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Divide;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Mod;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Multiply;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Subtract;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Average;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Max;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Min;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::EqualTo;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::Exponent;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::GreaterThan;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::GreaterThanEqual;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::LessThan;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::iMath::LessThanEqual;
	TCode->script = this;
	functions.Create(TCode);

	//fMath--------------------------------
	TCode = new SLStdFunctions::fMath::Add;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Divide;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Mod;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Multiply;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Subtract;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Average;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Max;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Min;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::EqualTo;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::Exponent;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::GreaterThan;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::GreaterThanEqual;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::LessThan;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::fMath::LessThanEqual;
	TCode->script = this;
	functions.Create(TCode);

	//Printng--------------------------------
	TCode = new SLStdFunctions::Printing::Print;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Printing::PrintInt;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Printing::PrintFloat;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Printing::PrintMix;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Printing::PrintFunctions;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Printing::PrintVariables;
	TCode->script = this;
	functions.Create(TCode);

	//Control Statements--------------------------------
	TCode = new SLStdFunctions::ControlStatements::For;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::ControlStatements::If;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::ControlStatements::Return;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::ControlStatements::While;
	TCode->script = this;
	functions.Create(TCode);

	//Strings--------------------------------
	TCode = new SLStdFunctions::Strings::Concat;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::ConcatFloat;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::ConcatInt;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::SetChar;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::SetFloatStringAcc;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::StringToInt;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::StringToFloat;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::StrFind;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::Strlen;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::StrCmp;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::Strcpy;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Strings::Replace;
	TCode->script = this;
	functions.Create(TCode);

	TCode = new SLStdFunctions::Windows::SendKeyboardKey;
	TCode->script = this;
	functions.Create(TCode);
}
