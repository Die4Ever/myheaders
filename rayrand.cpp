#include <oswrapper.h>
#include <rayranddecl.h>

DWORD		rrandtls=0;

RayRand64::RayRand64()
{
	if(Max==0)
	{
		Max = 0;
		Max-=5;
		Gen1 = Max / 3;
		Gen2 = Gen1;
		newseed.newseed64 = 10;
		Gen1 /= 2;
	}
	else newseed.newseed64 = 10;
	listnum=-1;
}

unsigned __int64 RayRand64::Gen1 = 0;
unsigned __int64 RayRand64::Gen2 = 0;
unsigned __int64 RayRand64::Max = 0;



RayRand32::RayRand32()
{
	if(Max==0)
	{
		Max = 0;
		Max-=5;
		Max = (unsigned int)(27312890881);//(128*3256*65535 + 1);
		Gen1 = Max / 3;
		Gen2 = Gen1;
		newseed = 10;
		Gen1 /= 2;
	}
	else newseed = 10;
}

unsigned __int32 RayRand32::Gen1 = 0;
unsigned __int32 RayRand32::Gen2 = 0;
unsigned __int32 RayRand32::Max = 0;
