class MathProblem
{
public:
	MathProblem* A;
	MathProblem* B;

	char operation;//+, -, *, /, %
	float NumA;
	float NumB;
	int set;//1 for NumA is set 2 for both are set

	MathProblem()
	{
		set = 0;
		operation = '\0';
		NumA = NumB = 0.0f;
		A = B = NULL;
	}

	~MathProblem()
	{
		if(A != NULL)
			delete A;
		if(B!=NULL)
			delete B;
	}

	float Operate()
	{
		switch(operation)
		{
		case '+':
			return NumA + NumB;
		case '-':
			return NumA - NumB;
		case '*':
			return NumA * NumB;
		case '/':
			if(NumB == 0)
				return 0;
			return NumA / NumB;
		case '%':
			if((int)NumB == 0)
				return 0;
			return fmod(NumA,NumB);
			return (int)NumA % (int)NumB;
		case '^':
			return pow(NumA,NumB);
		case 's':
			return sqrt(NumA);
		default:
			return NumA;
		}
	}

	float Solve(char* equation, int &i)
	{
		for(;i<2048;i++)
		{
			if(equation[i] == '\0' || equation[i] == ')' || equation[i] == ']' || equation[i] == '}')
				return Operate();
			else if(equation[i] == '(' || equation[i] == '[' || equation[i] == '{')
			{
				i++;
				if(set==0)
				{
					A = new MathProblem;
					NumA = A->Solve(equation,i);
					set=1;
				}
				else
				{
					B = new MathProblem;
					NumB = B->Solve(equation,i);
					set=2;
					//return Operate();
				}
			}
			else if(set==1 && operation == '\0' && (equation[i] == '+' || equation[i] == '-' || equation[i] == '*' || equation[i] == '/' || equation[i] == '%' || equation[i] == '^'))
			{
				operation = equation[i];
			}
			else if(set==1 && operation == '\0' && equation[i] == 's')
			{
				operation = equation[i];
				set = 2;
			}
			else if(equation[i] != ' ')
			{
				int a;
				//scanf_s can crash if it's not a number, try and catch?
				if(set==0)
				{
					sscanf_s(&equation[i],"%f%n",&NumA,&a);
					i += a-1;
					set=1;
				}
				else
				{
					sscanf_s(&equation[i],"%f%n",&NumB,&a);
					i+=a-1;
					//i++;//put it past the )
					set=2;
					//return Operate();
				}
			}
		}
	}
};