#include <SexyAppFramework/BassLoader.h>
#include <stdlib.h>

using namespace Sexy;

BASS_INSTANCE* Sexy::gBass = NULL;
static long gBassLoadCount = 0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CheckBassFunction(FARPROC theFunc, const char* theName)
{
	if (theFunc == nullptr)
	{
		char aBuf[1024];
		sprintf(aBuf, "%s function not found in bass.dll", theName);
		MessageBoxA(NULL, aBuf, "Error", MB_OK | MB_ICONERROR);
		exit(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BASS_INSTANCE::BASS_INSTANCE(const char* dllName)
{
	mModule = LoadLibrary(dllName);
	if (!mModule)
		return;

#define GETPROC(_x) CheckBassFunction(*((FARPROC*)&_x) = GetProcAddress(mModule, #_x), #_x)

	GETPROC(BASS_Init);
	GETPROC(BASS_Free);
	GETPROC(BASS_Stop);
	GETPROC(BASS_Start);
	GETPROC(BASS_SetVolume);
	GETPROC(BASS_SetConfig);
	GETPROC(BASS_GetConfig);

	GETPROC(BASS_GetVolume);
	GETPROC(BASS_GetInfo);

	GETPROC(BASS_GetVersion);
	GETPROC(BASS_ChannelStop);
	GETPROC(BASS_ChannelPlay);
	GETPROC(BASS_ChannelPause);
	GETPROC(BASS_ChannelSetAttribute);
	GETPROC(BASS_ChannelGetAttribute);
	GETPROC(BASS_ChannelSetPosition);
	GETPROC(BASS_ChannelGetPosition);
	GETPROC(BASS_ChannelIsActive);
	GETPROC(BASS_ChannelFlags);
	GETPROC(BASS_ChannelSlideAttribute);
	GETPROC(BASS_ChannelIsSliding);
	GETPROC(BASS_ChannelGetLevel);
	GETPROC(BASS_ChannelSetSync);
	GETPROC(BASS_ChannelRemoveSync);
	GETPROC(BASS_ChannelGetData);

	// supported by BASS 1.1 and higher. Only work if the user has DX8 or higher though.
	GETPROC(BASS_FXSetParameters);
	GETPROC(BASS_FXGetParameters);
	GETPROC(BASS_ChannelSetFX);
	GETPROC(BASS_ChannelRemoveFX);

	GETPROC(BASS_MusicLoad);
	GETPROC(BASS_MusicFree);

	GETPROC(BASS_StreamCreateFile);
	GETPROC(BASS_StreamFree);

	GETPROC(BASS_SampleLoad);
	GETPROC(BASS_SampleFree);
	GETPROC(BASS_SampleSetInfo);
	GETPROC(BASS_SampleGetInfo);
	GETPROC(BASS_SampleGetChannel);
	GETPROC(BASS_SampleStop);

	GETPROC(BASS_ErrorGetCode);
	GETPROC(BASS_PluginLoad);
	GETPROC(BASS_ChannelGetLength);
#undef GETPROC
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BASS_INSTANCE::~BASS_INSTANCE()
{
	if (mModule)
		FreeLibrary(mModule);
}

BOOL BASS_INSTANCE::BASS_MusicSetAmplify(HMUSIC handle, DWORD amp)
{
	BASS_MusicSetAttribute(handle, BASS_ATTRIB_MUSIC_AMPLIFY, amp);
	return true;
}

BOOL BASS_INSTANCE::BASS_MusicPlay(HMUSIC handle)
{
	return BASS_ChannelPlay(handle, true);
}

#define MAKEMUSICPOS(order,row) (0x80000000|MAKELONG(order,row))

BOOL BASS_INSTANCE::BASS_MusicPlayEx(HMUSIC handle, DWORD pos, int flags, BOOL reset)
{
	int anOffset = MAKEMUSICPOS(pos, 0);

	BASS_ChannelStop(handle);
	BASS_ChannelSetPosition(handle, anOffset, BASS_POS_MUSIC_ORDER);
	BASS_ChannelSetFlags(handle, flags);

	return BASS_ChannelPlay(handle, false /*reset*/);
}

BOOL BASS_INSTANCE::BASS_ChannelResume(DWORD handle)
{
	return BASS_ChannelPlay(handle, false);
}

BOOL BASS_INSTANCE::BASS_StreamPlay(HSTREAM handle, BOOL flush, DWORD flags)
{
	BASS_ChannelSetFlags(handle, flags);
	return BASS_ChannelPlay(handle, flush);
}

BOOL BASS_INSTANCE::BASS_ChannelSetAttributes(DWORD handle, int freq, int volume, int pan)
{
	BOOL result = BASS_ChannelSetAttribute(handle, BASS_ATTRIB_FREQ, (float)freq);
	if (result)
		result = BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, (float)volume / 100.0f);
	if (result)
		result = BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, (float)pan / 100.0f);
	return result;
}

BOOL BASS_INSTANCE::BASS_ChannelGetAttributes(DWORD handle, DWORD* freq, DWORD* volume, int* pan)
{
	float fVolume, fPan;
	BOOL result = BASS_ChannelGetAttribute(handle, BASS_ATTRIB_FREQ, (float*)freq);
	if (result)
		result = BASS_ChannelGetAttribute(handle, BASS_ATTRIB_VOL, &fVolume);
	if (result)
		result = BASS_ChannelGetAttribute(handle, BASS_ATTRIB_PAN, &fPan);
	if (result)
	{
		*volume = (DWORD)(fVolume * 100.0f);
		*pan = (int)(fPan * 100.0f);
	}
	return result;
}

BOOL BASS_INSTANCE::BASS_ChannelSetFlags(DWORD handle, DWORD flags)
{
	return BASS_ChannelFlags(handle, flags, 0xFFFFFFFF) != -1;
}

BOOL BASS_INSTANCE::BASS_ChannelSlideAttributes(DWORD handle, int freq, int volume, int pan, DWORD time)
{
	BOOL result = BASS_ChannelSlideAttribute(handle, BASS_ATTRIB_FREQ, (float)freq, time);
	if (result)
		result = BASS_ChannelSlideAttribute(handle, BASS_ATTRIB_VOL, (float)volume / 100.0f, time);
	if (result)
		result = BASS_ChannelSlideAttribute(handle, BASS_ATTRIB_PAN, (float)pan / 100.0f, time);
	return result;
}


DWORD BASS_INSTANCE::BASS_MusicGetAttribute(DWORD handle, DWORD attrib)
{
	float value;
	if (BASS_ChannelGetAttribute(handle, attrib, &value))
		return (DWORD)value;
	else
		return 0;
}

void BASS_INSTANCE::BASS_MusicSetAttribute(DWORD handle, DWORD attrib, DWORD value)
{
	float fValue = (float)value;
	BASS_ChannelSetAttribute(handle, attrib, fValue);
}


DWORD BASS_INSTANCE::BASS_MusicGetOrders(HMUSIC handle)
{
	return BASS_ChannelGetLength(handle, BASS_POS_MUSIC_ORDER);
}

DWORD BASS_INSTANCE::BASS_MusicGetOrderPosition(HMUSIC handle)
{
	return BASS_ChannelGetPosition(handle, BASS_POS_MUSIC_ORDER);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Sexy::LoadBassDLL()
{
	InterlockedIncrement(&gBassLoadCount);
	if (gBass != NULL)
		return;

	gBass = new BASS_INSTANCE(".\\bass.dll");
	if (gBass->mModule == NULL)
	{
		MessageBoxA(NULL, "Can't find bass.dll.", "Error", MB_OK | MB_ICONERROR);
		exit(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Sexy::FreeBassDLL()
{
	if (gBass != NULL)
	{
		if (InterlockedDecrement(&gBassLoadCount) <= 0)
		{
			delete gBass;
			gBass = NULL;
		}
	}
}
