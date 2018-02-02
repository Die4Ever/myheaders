#pragma once

#include <Mmreg.h>

#ifdef RAYHEADERSMAIN
//#include <iostream>
#include <fstream>
#include <wchar.h>
#include <windows.h>
#include <MMSYSTEM.h>
#include <vector>
#include <sstream>
using namespace std;
#include <rayrand.h>
#include <raycontainers.h>

/*#else
template <class A>
class ContObj;*/
#endif

class SoundBuffer : public ContObj<SoundBuffer>
{
public:
	HWAVEOUT SC;
	WAVEHDR WaveHeader;
	__int16 *WaveData;
	int done;
	int sent;
	int samples;

	int del;

	SoundBuffer(int samps, HWAVEOUT sc);

	virtual ~SoundBuffer();
};

void CALLBACK RaywaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

class RaySound
{
public:
	int OutBuffers;
	LinkedList<SoundBuffer> buffers;

	HWAVEOUT SC;
	WAVEFORMATEXTENSIBLE WaveFormat;
	CRITICAL_SECTION cs;
	int Channels;

	void QueueSB(SoundBuffer *sb);

	RaySound();
	RaySound(int channels, int samplerate, int bitdepth, UINT device);
	static bool Query(int channels, int samplerate, int bitdepth, UINT device);

	RaySound(RaySound &&rs)
	{
		SC=rs.SC;
		rs.SC=NULL;
		WaveFormat=rs.WaveFormat;
		Channels=rs.Channels;
		InitializeCriticalSection(&cs);
	}

	virtual ~RaySound();

	int Proc();
};

class SoundBufferIn : public ContObj<SoundBufferIn>
{
public:
	HWAVEIN SC;
	WAVEHDR WaveHeader;
	__int16 *WaveData;
	int done;
	int sent;
	int samples;

	int del;

	//SoundBufferIn();
	SoundBufferIn(int samps, HWAVEIN sc);
	virtual ~SoundBufferIn();
};

void CALLBACK RaywaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

class RaySoundIn
{
public:
	LinkedList<SoundBufferIn> buffers;

	HWAVEIN SC;
	WAVEFORMATEX WaveFormat;
	CRITICAL_SECTION cs;

	RaySoundIn();
	virtual ~RaySoundIn();

	SoundBufferIn *GetData()
	{
		EnterCriticalSection(&cs);
		SoundBufferIn *p = NULL;
		if(buffers.pHead && buffers.pHead->done==1)
		{
			p=buffers.pHead;
			buffers.Del(p);

			buffers.Create( new SoundBufferIn(512, SC) );
			waveInAddBuffer(SC, &buffers.pTail->WaveHeader, sizeof(WAVEHDR));
			LeaveCriticalSection(&cs);
			return p;
		}
		LeaveCriticalSection(&cs);
		return NULL;
	}
};


#ifdef RAYHEADERSMAIN
SoundBuffer::SoundBuffer(int samps, HWAVEOUT sc)
{
	SC = sc;
	del=0;
	samples = samps;
	done = 0;
	sent = 0;
	WaveData = new __int16[samps];
	memset(&WaveHeader, 0, sizeof(WaveHeader));
	WaveHeader.lpData = (char*)WaveData;
	WaveHeader.dwBufferLength = samps*2;
	//WaveHeader.dwFlags = WHDR_PREPARED;
	int Res = waveOutPrepareHeader( SC, &WaveHeader, sizeof(WAVEHDR));
	memset(WaveData, 0, samps*2);
}

SoundBuffer::~SoundBuffer()
{
	waveOutUnprepareHeader(SC, &WaveHeader, sizeof(WAVEHDR));
	delete[] WaveData;
}


void RaySound::QueueSB(SoundBuffer *sb)
{
	EnterCriticalSection(&cs);
	buffers.Create(sb);
	waveOutWrite(SC, &sb->WaveHeader, sizeof(WAVEHDR) );
	OutBuffers++;
	LeaveCriticalSection(&cs);
}

RaySound::RaySound()
{
	OutBuffers=0;
	InitializeCriticalSection(&cs);

	Channels = 1;
	int samplerate = 44100;
	int bytespersample = 2;
	WaveFormat.Format.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.Format.nChannels = 1;
	WaveFormat.Format.cbSize = 0;
	WaveFormat.Format.nAvgBytesPerSec = samplerate * bytespersample;
	WaveFormat.Format.nBlockAlign = bytespersample;
	WaveFormat.Format.nSamplesPerSec = samplerate;
	WaveFormat.Format.wBitsPerSample = bytespersample*8;
	
	WaveFormat.Samples.wValidBitsPerSample = bytespersample*8;

	int e = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat.Format, 0, 0, WAVE_FORMAT_QUERY);

	int Res = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat.Format, (DWORD_PTR)&RaywaveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION/* | WAVE_ALLOWSYNC*/);
	e = GetLastError();
}

RaySound::RaySound(int channels, int samplerate, int bitdepth, UINT device)
{
	OutBuffers=0;
	InitializeCriticalSection(&cs);

	Channels = channels;
	//int samplerate = 44100;
	int bytespersample = bitdepth/8;
	WaveFormat.Format.wFormatTag = WAVE_FORMAT_PCM;//WAVE_FORMAT_EXTENSIBLE;
	WaveFormat.Format.nChannels = channels;
	WaveFormat.Format.cbSize = 0;//22;
	WaveFormat.Format.nAvgBytesPerSec = channels * samplerate * bytespersample;
	WaveFormat.Format.nBlockAlign = bytespersample * channels;
	WaveFormat.Format.nSamplesPerSec = /*channels * */samplerate;
	WaveFormat.Format.wBitsPerSample = bytespersample*8;

	WaveFormat.Samples.wValidBitsPerSample = bytespersample*8;
	//WaveFormat.Samples.wReserved = 0;
	//WaveFormat.Samples.wSamplesPerBlock = 8;

	WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	
	if(channels == 1)
		WaveFormat.dwChannelMask = SPEAKER_FRONT_CENTER;
	if(channels >= 2)
		WaveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	if(channels == 3)
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER;
	if(channels >= 4)
		WaveFormat.dwChannelMask |= SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
	if(channels == 6)//5.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY;
	if(channels == 7)//6.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER;
	if(channels == 8)//7.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;

	int e = waveOutOpen(&SC, device, &WaveFormat.Format, 0, 0, WAVE_FORMAT_QUERY);

	int Res = waveOutOpen(&SC, device, &WaveFormat.Format, (DWORD_PTR)&RaywaveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION/* | WAVE_ALLOWSYNC*/);
	e = GetLastError();
}

bool RaySound::Query(int channels, int samplerate, int bitdepth, UINT device)
{
	WAVEFORMATEXTENSIBLE WaveFormat;

	int bytespersample = bitdepth/8;
	WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	WaveFormat.Format.nChannels = channels;
	WaveFormat.Format.cbSize = 22;
	WaveFormat.Format.nAvgBytesPerSec = channels * samplerate * bytespersample;
	WaveFormat.Format.nBlockAlign = bytespersample * channels;
	WaveFormat.Format.nSamplesPerSec = channels * samplerate;
	WaveFormat.Format.wBitsPerSample = bytespersample*8;

	WaveFormat.Samples.wValidBitsPerSample = bytespersample*8;
	//WaveFormat.Samples.wReserved = 0;
	//WaveFormat.Samples.wSamplesPerBlock = 0;

	WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	
	if(channels == 1)
		WaveFormat.dwChannelMask = SPEAKER_FRONT_CENTER;
	if(channels >= 2)
		WaveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	if(channels == 3)
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER;
	if(channels >= 4)
		WaveFormat.dwChannelMask |= SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
	if(channels == 6)//5.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY;
	if(channels == 7)//6.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER;
	if(channels == 8)//7.1
		WaveFormat.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;

	int e = waveOutOpen(NULL, device, &WaveFormat.Format, 0, 0, WAVE_FORMAT_QUERY);
	return e == MMSYSERR_NOERROR;
}

RaySound::~RaySound()
{
	while(Proc())
		Sleep(1);
	Sleep(100);

	if(SC!=NULL)
		waveOutClose(SC);
	DeleteCriticalSection(&cs);
}

int RaySound::Proc()
{
	EnterCriticalSection(&cs);
	for(SoundBuffer *sb=buffers.pHead; sb!=NULL; sb=sb->pNext)
	{
		if(sb->del == 1)
		{
			//OutBuffers--;
			delete sb;
			break;
		}
	}
	int ob = OutBuffers;
	LeaveCriticalSection(&cs);
	//if(OutBuffers > 0)
	//	return 1;
	//else
	//	return 0;
	return ob;
}


void CALLBACK RaywaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if(uMsg == WOM_DONE)
	{
		//MakeNewOutBuffer++;
		if(dwInstance != NULL)
		{
			EnterCriticalSection( &((RaySound*)dwInstance)->cs);
			//if(((RaySound*)dwInstance)->buffers.pHead!=NULL)
				//((RaySound*)dwInstance)->buffers.pHead->del = 1;
			for(SoundBuffer *sb = ((RaySound*)dwInstance)->buffers.pHead; sb; sb=sb->pNext)
				if(sb->del==0)
				{
					sb->del=1;
					break;
				}
			((RaySound*)dwInstance)->OutBuffers--;
			LeaveCriticalSection( &((RaySound*)dwInstance)->cs);
		}
		//delete SoundOutBuffers.pHead;
	}
}

RaySoundIn::RaySoundIn()
{
	int samplerate = 44100;
	int bytespersample = 2;
	int channels=1;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = channels;
	WaveFormat.cbSize = 0;
	WaveFormat.nAvgBytesPerSec = samplerate * bytespersample * channels;
	WaveFormat.nBlockAlign = bytespersample;
	WaveFormat.nSamplesPerSec = samplerate * channels;
	WaveFormat.wBitsPerSample = bytespersample*8;

	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
	int e = waveInOpen(&SC, WAVE_MAPPER, &WaveFormat, 0, 0, WAVE_FORMAT_QUERY);

	int Res = waveInOpen(&SC, WAVE_MAPPER, &WaveFormat, (DWORD_PTR)&RaywaveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION/* | WAVE_ALLOWSYNC*/);
	e = GetLastError();

	e=waveInStart(SC);
	buffers.Create( new SoundBufferIn(512, SC) );
	e=waveInAddBuffer(SC, &buffers.pHead->WaveHeader, sizeof(WAVEHDR));

	buffers.Create( new SoundBufferIn(512, SC) );
	e=waveInAddBuffer(SC, &buffers.pTail->WaveHeader, sizeof(WAVEHDR));

	buffers.Create( new SoundBufferIn(512, SC) );
	e=waveInAddBuffer(SC, &buffers.pTail->WaveHeader, sizeof(WAVEHDR));
	LeaveCriticalSection(&cs);
}

RaySoundIn::~RaySoundIn()
{
	waveInClose(SC);
	DeleteCriticalSection(&cs);
}

void CALLBACK RaywaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	RaySoundIn *pthis = (RaySoundIn*)dwInstance;
	if(uMsg == WIM_DATA)
	{
		EnterCriticalSection(&pthis->cs);
		for(SoundBufferIn *p=pthis->buffers.pHead;p;p=p->pNext)
		{
			if(p->done==0)
			{
				p->done=1;
				break;
			}
		}
		LeaveCriticalSection(&pthis->cs);
	}
}

SoundBufferIn::SoundBufferIn(int samps, HWAVEIN sc)
{
	SC = sc;
	del=0;
	samples = samps;
	done = 0;
	sent = 0;
	WaveData = new __int16[samps];
	memset(&WaveHeader, 0, sizeof(WaveHeader));
	WaveHeader.lpData = (char*)WaveData;
	WaveHeader.dwBufferLength = samps*2;
	//WaveHeader.dwFlags = WHDR_PREPARED;
	int Res = waveInPrepareHeader( SC, &WaveHeader, sizeof(WAVEHDR));
	memset(WaveData, 0, samps*2);
}

SoundBufferIn::~SoundBufferIn()
{
	waveInUnprepareHeader(SC, &WaveHeader, sizeof(WAVEHDR));
	delete[] WaveData;
}
#endif
