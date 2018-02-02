union TYPES
{
	__int64 i64[2];//16 bytes to match the Key for my DB
	__int32 i32[4];
	__int16 i16[8];
	__int8	i8[16];
	char	c8[16];//can't hurt, helps with string functions
};
TYPES uTypes;