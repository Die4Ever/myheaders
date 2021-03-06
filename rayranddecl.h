#pragma once
#ifdef WIN32
#include <windows.h>
#endif

//const unsigned __int64 All1s64 = ~0;
//const unsigned __int32 All1s32 = ~0;
const unsigned __int64 All1s64 = ~0;
const unsigned __int32 All1s32 = ~0;

extern DWORD rrandtls;

class RayRand64
{
public:
	//unsigned __int64 seed;
	union unewseed
	{
		unsigned __int64 newseed64;
		unsigned __int32 newseed32[2];
		unsigned __int16 newseed16[4];
		unsigned __int8 newseed8[8];
	};
	unewseed newseed;//note: the lowest order byte has slightly worse distribution across the values than the rest, but it is still acceptable for most uses if you want high quality random numbers, don't use the 0 index of the arrays, example for a 32 bit number use newseed32[1], or for 16 bit, use anything but 0, and for 8 bit use anything but 0 if you need a higher range than 32 bit, but you need the extra quality of the higher bits, you can use newseed64 and do a right shift by 8 bits (newseed64 >> 8) to shift it down a byte, this will give you an unsigned 56 bit range (0 - 72,057,594,037,927,935) (the 64 bit range is 0 - 18,446,744,073,709,551,615)
	static unsigned __int64 Gen1;
	static unsigned __int64 Gen2;
	static unsigned __int64 Max;

	int listnum;

	__inline unsigned __int64 iRand64HQ();
	
	__inline unsigned __int64 iRand64();

	__inline unsigned __int64 iRand64HQOld();

	__inline unsigned __int64 iRand64Old();

	__inline void SetStrongSeed(unsigned __int64 iseed);//use this if you use the time function to set the seed, because the time function has a low range

	RayRand64();

	__inline unsigned __int32 RandList32();//when compiling to a 32 bit exe, this is slower than just calling the iRand32() function for each number
};

/*unsigned __int64 RayRand64::Gen1 = 0;
unsigned __int64 RayRand64::Gen2 = 0;
unsigned __int64 RayRand64::Max = 0;*/

class RayRand32
{
public:
	//unsigned __int32 seed;
	unsigned __int32 newseed;
	static unsigned __int32 Gen1;
	static unsigned __int32 Gen2;
	static unsigned __int32 Max;

	__inline unsigned __int32 iRand32();

	__inline unsigned __int32 iRand32HQ();

	__inline unsigned __int32 iRand32Old();

	void SetStrongSeed(unsigned __int32 iseed);

	RayRand32();
};

/*unsigned __int32 RayRand32::Gen1 = 0;
unsigned __int32 RayRand32::Gen2 = 0;
unsigned __int32 RayRand32::Max = 0;*/


__inline unsigned __int32 RayRand32::iRand32()
{
	//seed = newseed;
	newseed = Gen1 * newseed * 5 + Gen2 + (newseed/5) * 3  + ((newseed>>24) & 0xFF);
	return newseed;
}

__inline unsigned __int32 RayRand32::iRand32HQ()
{
	//seed = newseed;
	newseed = Gen1 * newseed * 5 + Gen2 + (newseed/5) * 3  + ((newseed>>24) & 0xFF);
	return newseed>>8;
}

__inline unsigned __int32 RayRand32::iRand32Old()
{
	//seed = newseed;
	newseed = Gen1 * newseed * 5 + Gen2 + (newseed/5) * 3;
	return newseed;
}

__inline void RayRand32::SetStrongSeed(unsigned __int32 iseed)
{
	newseed = iseed;
	iRand32();
	iRand32();
	iRand32();
}



__inline unsigned __int32 RayRand64::RandList32()//when compiling to a 32 bit exe, this is slower than just calling the iRand32() function for each number
{
	if(listnum<1)
	{
		listnum = 2;
		iRand64();
		return (unsigned __int32)((newseed.newseed64 ^ All1s64)>>9);
	}
	listnum--;
	return newseed.newseed32[listnum];
}

__inline unsigned __int64 RayRand64::iRand64HQ()
{
	//seed = newseed.newseed64;
	newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3 + ((newseed.newseed64>>24) & 0xFF);// + newseed.newseed8[2];
	return newseed.newseed64>>8;
}
__inline unsigned __int64 RayRand64::iRand64()
{
	//seed = newseed.newseed64;
	newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3 + ((newseed.newseed64>>24) & 0xFF);// + newseed.newseed8[2];
	return newseed.newseed64;
}

__inline unsigned __int64 RayRand64::iRand64HQOld()
{
	//seed = newseed.newseed64;
	newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3;
	return newseed.newseed64>>8;
}

__inline unsigned __int64 RayRand64::iRand64Old()
{
	//seed = newseed.newseed64;
	newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3;
	return newseed.newseed64;
}

__inline void RayRand64::SetStrongSeed(unsigned __int64 iseed)//use this if you use the time function to set the seed, because the time function has a low range
{
	newseed.newseed64 = iseed;

	iRand64();
	iRand64();
	iRand64();
}

