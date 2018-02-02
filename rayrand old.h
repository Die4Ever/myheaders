/*class RayRand128//not supported on x64 :(
{
public:
	unsigned __int128 seed;
	union unewseed
	{
		unsigned __int128 newseed128;
		unsigned __int64 newseed64[2];
		unsigned __int32 newseed32[4];
		unsigned __int16 newseed16[8];
		unsigned __int8 newseed8[16];
	};
	unewseed newseed;
	unsigned __int128 Gen1;
	unsigned __int128 Gen2;
	unsigned __int128 Max;
	unsigned __int128 iRand128()
	{
		seed = newseed.newseed128;
		newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (seed/5) * 3;
		return newseed.newseed128;
	}

	void SetSrongSeed(unsigned __int128 iseed)
	{
		newseed.newseed128 = iseed;

		iRand128();
		iRand128();
		iRand128();
	}

	RayRand128()
	{
		Max = 0;
		Max-=5;
		Gen1 = Max / 3;
		Gen2 = Gen1;
		newseed.newseed128 = 10;
		Gen1 /= 2;
	}
};*/
const unsigned __int64 All1s64 = ((signed __int64)-1);
const unsigned __int32 All1s32 = ((signed __int32)-1);

class RayRand64
{
public:
	unsigned __int64 seed;
	union unewseed
	{
		unsigned __int64 newseed64;
		unsigned __int32 newseed32[2];
		unsigned __int16 newseed16[4];
		unsigned __int8 newseed8[8];
	};
	unewseed newseed;//note: the lowest order byte has slightly worse distribution across the values than the rest, but it is still acceptable for most uses if you want high quality random numbers, don't use the 0 index of the arrays, example for a 32 bit number use newseed32[1], or for 16 bit, use anything but 0, and for 8 bit use anything but 0 if you need a higher range than 32 bit, but you need the extra quality of the higher bits, you can use newseed64 and do a right shift by 8 bits (newseed64 >> 8) to shift it down a byte, this will give you an unsigned 56 bit range (0 - 72,057,594,037,927,935) (the 64 bit range is 0 - 18,446,744,073,709,551,615, and yes it does hit all those values)
	unsigned __int64 Gen1;
	unsigned __int64 Gen2;
	unsigned __int64 Max;

	int listnum;

	__inline unsigned __int64 iRand64HQ()
	{
		//seed = newseed.newseed64;
		newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3;
		return newseed.newseed64>>8;
	}
	__inline unsigned __int64 iRand64()
	{
		//seed = newseed.newseed64;
		newseed.newseed64 = Gen1 * newseed.newseed64 * 5 + Gen2 + (newseed.newseed64/5) * 3;
		return newseed.newseed64;
	}

	__inline void SetSrongSeed(unsigned __int64 iseed)//use this if you use the time function to set the seed, because the time function has a low range
	{
		newseed.newseed64 = iseed;

		iRand64();
		iRand64();
		iRand64();
	}

	RayRand64()
	{
		Max = 0;
		Max-=5;
		Gen1 = Max / 3;
		Gen2 = Gen1;
		newseed.newseed64 = 10;
		Gen1 /= 2;
		listnum=-1;
	}

	__inline unsigned __int32 RandList32()//when compiling to a 32 bit exe, this is slower than just calling the iRand32() function for each number
	{
		if(listnum<1)
		{
			listnum = 2;
			iRand64();
			return ((newseed.newseed64 ^ All1s64)>>9);
		}
		listnum--;
		return newseed.newseed32[listnum];
	}
};

class RayRand32
{
public:
	//unsigned __int32 seed;
	unsigned __int32 newseed;
	unsigned __int32 Gen1;
	unsigned __int32 Gen2;
	unsigned __int32 Max;
	unsigned __int32 iRand32()
	{
		//seed = newseed;
		newseed = Gen1 * newseed * 5 + Gen2 + (newseed/5) * 3;
		return newseed;
	}

	void SetSrongSeed(unsigned __int32 iseed)
	{
		newseed = iseed;
		iRand32();
		iRand32();
		iRand32();
	}

	RayRand32()
	{
		Max = 0;
		Max-=5;
		Gen1 = Max / 3;
		Gen2 = Gen1;
		newseed = 10;
		Gen1 /= 2;
	}
};