/*struct PageInfo
{
unsigned char used;
};*/

class RayHeap
{
public:
	char *pEnd;
	char *pPages;
	unsigned int PageSize;
	unsigned int Pages;
	char *pi;
	unsigned int TFree;
	int iFree;

	RayHeap(int iPageSize, int iPages)
	{
		TFree=0;
		PageSize = iPageSize;
		iFree = Pages = iPages;

		pi = new char[Pages];
		memset(pi, 0, Pages);
		//for(int i=0;i<Pages)
		{
			pPages = new char[PageSize*Pages];
			pEnd = &pPages[PageSize*Pages];
		}
	}

	RayHeap()
	{
		TFree=0;
		PageSize = 16*1024;
		PageSize = 1024;
		iFree = Pages = 8192;

		pi = new char[Pages];
		memset(pi, 0, Pages);
		//for(int i=0;i<Pages)
		{
			pPages = new char[PageSize*Pages];
			pEnd = &pPages[PageSize*Pages];
		}
	}

	~RayHeap()
	{
		delete[] pi;
		//for(int i=0;i<Pages)
		{
			delete[] pPages;
		}
	}

	char* AllocBytes(unsigned int bytes)
	{
		__asm
		{
			/*move values into registers...
				EAX = TFree
				EBX = i
				ECX = this
				EDX = iFree
				ESI = pi
				EDI = Pages
				//MM0 = pPages
				//MM1 = PageSize

				EBP = PageSize
			*/
			push ebp

			//edx//bytes
			mov edx, DWORD PTR [bytes]

			cmp edx, ebp//if(bytes > PageSize) return NULL;
				ja SHORT LN1AllocBytes

			mov edi, [Pages+ecx]
			mov edx, [iFree+ecx]
			mov ebp, [PageSize+ecx]
			mov eax, [TFree+ecx]
			mov esi, [pi+ecx]

				//if( iFree<0) return NULL;
			cmp	edx, 0
				jle	SHORT LN1AllocBytes

			dec edx//iFree--
				mov DWORD PTR [ecx+iFree], edx

				//if(pi[TFree] == 0)
			cmp	BYTE PTR [esi+eax], 0
				jne	SHORT LN4AllocBytes//jump if false

				mov ebx, eax//i = TFree
			cmp	eax, edi//if(TFree+1 < Pages)
			ja	SHORT LN3AllocBytes

				add eax, 2//TFree += 2(I decrement it right after, just to avoid a jump)
LN3AllocBytes:
			dec	eax

LN2AllocBytes:
			mov DWORD PTR [ecx+TFree], eax

			mov	BYTE PTR [esi+ebx], 1

			//	mov	eax, ebp
			imul	ebp, ebx
			
			add	ebp, [ecx+pPages]
			mov eax, ebp
			jmp endfunc



LN4AllocBytes:

			/*move values into registers...
				EAX = TFree
				EBX = i
				ECX = this
				EDX = iFree
				ESI = pi
				EDI = Pages
				//MM0 = pPages
				//MM1 = PageSize

				EBP = PageSize
			*/

			; 147  : 			unsigned int i = strlen((char*)pi);
			mov edx, esi
LL12AllocBytes:
			mov bl, BYTE PTR [edx]
			inc edx
				test bl, bl
				jne SHORT LL12AllocBytes
				mov ebx, edx
				sub ebx, esi
				dec ebx

				//; 148  : 			if(i<Pages)
				lea	eax, DWORD PTR [ebx+1]
			cmp	edi, eax
				sbb	eax, eax
				neg	eax
				add	eax, ebx
				
				mov DWORD PTR [ecx+TFree], eax

				mov	BYTE PTR [edx], 1

			imul	ebp, ebx

			add	ebx, [ecx+pPages]
			mov eax, ebx

LN1AllocBytes:
				xor	eax, eax

endfunc:
				pop ebp
		}
		/*if(bytes < PageSize && iFree>0)
		{
			if(pi[TFree] == 0)
			{
				if(TFree+1<Pages)
					TFree++;
				else
					TFree--;

				pi[TFree]=1;
				iFree--;
				return &pPages[TFree*PageSize];
			}

			unsigned int i = strlen((char*)pi);
			if(i<Pages)
			{
				TFree = i + ( (i+1) <Pages);
				pi[i]=1;
				iFree--;
				return &pPages[i*PageSize];
			}
		}
		return NULL;*/
		//return new char[bytes];
	}

	void Dealloc(char *cptr)
	{
		__asm
		{
			; _this$ = ecx
			; _ptr$ = eax
			/*
			EBX = pPages
			EAX = ptr
			EDX = pi
			*/

			mov eax, DWORD PTR [cptr]

			mov	ebx, DWORD PTR [ecx+pPages]

			; 175  : 		if(ptr>=pPages && ptr<pEnd)

			cmp	eax, DWORD PTR [ecx]//pEnd
				jae	SHORT LN2Dealloc
			cmp	eax, ebx
				jb	SHORT LN2Dealloc

			; 177  : 			unsigned int i = ((unsigned int)(ptr-pPages))/PageSize;

			sub	eax, ebx
				xor	edx, edx
				div	DWORD PTR [ecx+PageSize]

			; 178  : 			pi[i]=0;

			//EAX = i

			mov	edx, DWORD PTR [ecx+pi]
			mov	BYTE PTR [eax+edx], 0

				; 179  : 			if(i<TFree)

			cmp	eax, DWORD PTR [ecx+TFree]
				jae	SHORT LN1Dealloc

				; 180  : 				TFree=i;

			mov	DWORD PTR [ecx+TFree], eax
LN1Dealloc:

			; 181  : 			iFree++;

			inc	DWORD PTR [ecx+iFree]
LN2Dealloc:
		}
		/*if(ptr>=pPages && ptr<pEnd)
		{
			unsigned int i = ((unsigned int)(ptr-pPages))/PageSize;
			pi[i]=0;
			if(i<TFree)
				TFree=i;
			iFree++;
			return;
		}*/
		//delete[] ptr;
	}
};

/*char* RayHeap::AllocBytes(unsigned int bytes)
{
if(bytes < PageSize && iFree>0)
{
if(pi[TFree] == 0)
{
if(TFree+1<Pages)
TFree++;
else
TFree--;

pi[TFree]=1;
iFree--;
return &pPages[TFree*PageSize];
}

unsigned int i = strlen((char*)pi);
if(i<Pages)
{
TFree = i + ( (i+1) <Pages);
pi[i]=1;
iFree--;
return &pPages[i*PageSize];
}
}
return NULL;
//return new char[bytes];
}*/

#define RayNew(heap, obj, args) new ((obj *)heap.AllocBytes( sizeof(obj))) obj args

#define RayNewA(heap, obj, count) new ((obj*)heap.AllocBytes( sizeof(obj) * count)) obj[count]()
#define RayDelete(heap, obj, ptr, args) ptr->~obj args, heap.Dealloc((char*)ptr), ptr=0