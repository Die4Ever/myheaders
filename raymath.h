int log(int a, int base)
{
	int i=16;
	//for(; i>1 && (a & (1<<i)) == 0; i--);
	int b = 1;
	for(i=0;i<30 && b*base<=a;i++)
		b *= base;

	return i;
}

int pow(int base, int exp)
{
	int res = 1;
	for(; exp>0; exp--)
		res *= base;
	return res;
}