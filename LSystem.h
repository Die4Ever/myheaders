class LSystemRule
{
public:
	string predecessor;
	string successor;
	float prob;

	LSystemRule(string p, string s, float Probability) : predecessor(p), successor(s)
	{
		prob=Probability;
	}

	string Apply(const char *input)
	{
		if( memcmp(input, predecessor.c_str(), predecessor.length())==0 )
		{
			if( rrand.iRand64HQ()%10000 < (prob*10000.0f) )
				return successor;
		}
		return string();
	}
};

class LSystem
{
public:
	vector<LSystemRule> rules;
	string state;
	uint gen;

	LSystem(string Axiom, string sRules) : state(Axiom)
	{
		gen=0;
		vector<string> srules;
		SplitBy(sRules.c_str(), ';', srules);
		for(uint i=0;i<srules.size()-1;i++)
		{
			vector<string> rule;
			SplitBy(srules[i].c_str(), '=', rule);
			float prob=1.0f;
			if(rule.size()==3)
				prob=ToFloat(rule[2].c_str());

			rules.push_back( LSystemRule(rule[0], rule[1], prob) );
		}
	}

	void AddRule(string sRules)
	{
		vector<string> srules;
		SplitBy(sRules.c_str(), ';', srules);
		for(uint i=0;i<srules.size()-1;i++)
		{
			vector<string> rule;
			SplitBy(srules[i].c_str(), '=', rule);
			float prob=1.0f;
			if(rule.size()==3)
				prob=ToFloat(rule[2].c_str());

			rules.push_back( LSystemRule(rule[0], rule[1], prob) );
		}
	}

	void Proc()
	{
		gen++;
		string newstate;
		for(uint i=0;i<state.length();i++)
		{
			uint oldlen = newstate.length();
			for(uint r=0;r<rules.size();r++)
			{
				newstate += rules[r].Apply( &state.c_str()[i] );
				if(oldlen != newstate.length())
					break;
			}
			if(oldlen == newstate.length())
				newstate += state[i];
		}
		state=newstate;
	}
};
