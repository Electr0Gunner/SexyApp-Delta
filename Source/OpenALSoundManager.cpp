#include <SexyAppFramework/OpenALSoundManager.h>

#include <SexyAppFramework/PakLib/PakInterface.h>
#include <SexyAppFramework/Debug.h>
#include <SexyAppFramework/OpenALSoundInstance.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

using namespace Sexy;

#include <thread>
#include <atomic>

OpenALSoundManager::OpenALSoundManager()
{
	mLastReleaseTick = 0;

	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSoundBuffers[i] = 0;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	for (int i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = nullptr;

    mMasterVolume = 1.0;

    mSoundDevice = alcOpenDevice(nullptr);
    if (!mSoundDevice)
    {
        return;
    }

    mSoundContext = alcCreateContext(mSoundDevice, nullptr);
    if (!alcMakeContextCurrent(mSoundContext))
    {
        return;
    }
}

OpenALSoundManager::~OpenALSoundManager()
{
    ReleaseChannels();
	ReleaseSounds();

    alcDestroyContext(mSoundContext);
    alcCloseDevice(mSoundDevice);

}

bool OpenALSoundManager::Initialized()
{
	return (mSoundContext != NULL && mSoundDevice != NULL);
}

// ----- Property control -----

int OpenALSoundManager::VolumeToDB(double theVolume)
{
	int aVol = (int)((log10(1 + theVolume * 9) - 1.0) * 2333);
	if (aVol < -2000)
		aVol = -10000;

	return aVol;
}

void OpenALSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

bool OpenALSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool OpenALSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}


double OpenALSoundManager::GetMasterVolume()
{
	return mMasterVolume;
}

void OpenALSoundManager::SetMasterVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i])
		{
			mPlayingSounds[i]->RehupVolume();
		}
	}

}

void OpenALSoundManager::Flush() {}

void OpenALSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed) {}

// ----- Sound IDs -----

int OpenALSoundManager::GetFreeSoundId()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (mSoundBuffers[i] == NULL)
			return i;
	}

	return -1;
}

int OpenALSoundManager::GetNumSounds()
{
	int aCount = 0;
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (mSoundBuffers[i] != 0)
			aCount++;
	}

	return aCount;
}

int OpenALSoundManager::FindFreeChannel()
{
	DWORD aTick = GetTickCount();
	if (aTick - mLastReleaseTick > 1000)
	{
		ReleaseFreeChannels();
		mLastReleaseTick = aTick;
	}

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] == NULL)
			return i;

		if (mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
			return i;
		}
	}

	return -1;
}

SoundInstance* OpenALSoundManager::GetSoundInstance(unsigned int theSfxID)
{
    if (!Initialized()) //Don't even bother if OpenAL didn't initialize.
        return nullptr;

	if (theSfxID > MAX_SOURCE_SOUNDS)
		return nullptr;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return nullptr;

	if (mSoundBuffers[theSfxID] == NULL)
		return nullptr;

	mPlayingSounds[aFreeChannel] = new OpenALSoundInstance(this, mSoundBuffers[theSfxID]);

	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

bool OpenALSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	if (!mSoundDevice)
		return true; // sounds just	won't play, but this is not treated as a failure condition

	mSourceFileNames[theSfxID] = theFilename;

	std::string aFilename = theFilename;
	std::string aCachedName;

    //TODO: add caching back
    /*
	if ((aFilename.length() > 2) && (aFilename[0] != '\\') && (aFilename[0] != '/') && (aFilename[1] != ':'))
	{
		// Not an absolute path
		aCachedName = GetAppDataFolder() + "cached\\" + aFilename + ".wav";
		if (LoadWAVSound(theSfxID, aCachedName))
			return true;
		MkDir(GetFileDir(aCachedName));
	}
    */

	if (DecodeSound(theSfxID, aFilename))
		return true;

	return false;
}

int OpenALSoundManager::LoadSound(const std::string& theFilename)
{
	int i;
	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i] == theFilename)
			return i;

	for (i = MAX_SOURCE_SOUNDS - 1; i >= 0; i--)
	{
		if (mSoundBuffers[i] == NULL)
		{
			if (!LoadSound(i, theFilename))
				return -1;
			else
				return i;
		}
	}

	return -1;
}

// ----- RELEASING -----

void OpenALSoundManager::ReleaseSound(unsigned int theSfxID)
{
	if (mSoundBuffers[theSfxID] != 0)
	{
        alDeleteBuffers(1, &mSoundBuffers[theSfxID]);
		mSoundBuffers[theSfxID] = 0;
		mSourceFileNames[theSfxID] = "";
	}
}

void OpenALSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		ReleaseSound(i);
}

void OpenALSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void OpenALSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL && mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void OpenALSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}

// ----- Reading/Decoding -----

bool OpenALSoundManager::DecodeSound(unsigned int theSfxID, const std::string& theFilename)
{
    if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

    if (DecodeOGGFormat(theSfxID, theFilename) || DecodeAUFormat(theSfxID, theFilename))
        return true;

    std::vector<std::string> aPossibleExtensions = {".mp3", ".flac", ".wav"};

    for (auto& aExt : aPossibleExtensions)
    {
        PFILE *fp = p_fopen((theFilename + aExt).c_str(), "rb");
		if (!fp)
			continue;

		p_fseek(fp, 0, SEEK_END);
		size_t fileSize = p_ftell(fp);
		p_fseek(fp, 0, SEEK_SET);
		uint8_t *data = new uint8_t[fileSize];
		p_fread(data, 1, fileSize, fp);
		p_fclose(fp);

		ma_decoder decoder;
		ma_result result = ma_decoder_init_memory(data, fileSize, NULL, &decoder);

        if (result != MA_SUCCESS)
		{
			delete[] data;
			continue;
		}

		ma_uint64 aFrameCount; ma_decoder_get_length_in_pcm_frames(&decoder, &aFrameCount); 
		size_t aSampleCount = static_cast<size_t>(aFrameCount) * decoder.outputChannels; 
		std::vector<int16_t> pcmData(aSampleCount);
   
		uint64_t aFrameReadCount = 0; 
		while (aFrameReadCount < aFrameCount) 
		{ 
			uint64_t framesRead = 0;
			ma_result res = ma_decoder_read_pcm_frames(
				&decoder,
				pcmData.data() + (aFrameReadCount * decoder.outputChannels), 
				aFrameCount - aFrameReadCount, 
				&framesRead
			); 
			if (res != MA_SUCCESS) 
				break; 
			if (framesRead == 0) 
				break; // EOF 
			aFrameReadCount += framesRead; 
		}

		size_t aTotalBytes = aFrameReadCount * decoder.outputChannels * sizeof(int16_t);

		ALenum aFormat = AL_NONE;
		if (decoder.outputChannels == 1) 
			aFormat = AL_FORMAT_MONO16;
		else if (decoder.outputChannels == 2) 
			aFormat = AL_FORMAT_STEREO16;
		else {
			delete[] data;
			ma_decoder_uninit(&decoder);
			continue;
		}

       	ALuint buffer; alGenBuffers(1, &buffer); 
		alBufferData(buffer, aFormat, pcmData.data(), pcmData.size() * sizeof(int16_t), decoder.outputSampleRate);
		mSoundBuffers[theSfxID] = buffer;
        mSourceFileNames[theSfxID] = theFilename;
		mSourceDataSizes[theSfxID] = pcmData.size() * sizeof(int16_t);
        
		delete[] data;
		ma_decoder_uninit(&decoder);
		return true;

    }

	return false;
}

static int p_fseek64_wrap(PFILE *f, ogg_int64_t off, int whence)
{
	if (!f)
		return -1;

	return p_fseek(f, (long)off, whence);
}

int ov_pak_open(PFILE *f, OggVorbis_File *vf, char *initial, long ibytes)
{
	ov_callbacks callbacks = {(size_t(*)(void *, size_t, size_t, void *))p_fread,
							  (int (*)(void *, ogg_int64_t, int))p_fseek64_wrap, (int (*)(void *))p_fclose,
							  (long (*)(void *))p_ftell};

	return ov_open_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

bool OpenALSoundManager::DecodeOGGFormat(unsigned int theSfxID, const std::string& theFilename)
{
	OggVorbis_File vf;
	int current_section;

    PFILE *aFile = p_fopen((theFilename + ".ogg").c_str(), "rb");
	if (!aFile)
		return false;

	if (ov_pak_open(aFile, &vf, NULL, 0) < 0)
	{
		p_fclose(aFile);
		return false;
	}

	vorbis_info* anInfo = ov_info(&vf, -1);

	ALenum aFormat;
	if (anInfo->channels == 1)
		aFormat = AL_FORMAT_MONO16;
	else if (anInfo->channels == 2)
		aFormat = AL_FORMAT_STEREO16;
	else
	{
		ov_clear(&vf);
		p_fclose(aFile);
		return false;
	}

    int aLenBytes = static_cast<int>(ov_pcm_total(&vf, -1) * anInfo->channels * 2);

	char *aBuf = new char[aLenBytes]; // temp buffer

	char *aPtr = aBuf;
	int aNumBytes = aLenBytes;
	while (aNumBytes > 0)
	{
    	long ret = ov_read(&vf, aPtr, aNumBytes, 0, 2, 1, &current_section);

		if (ret == 0)
			break;
		else if (ret < 0)
		{
			// this means something is wrong
			delete[] aBuf;
			ov_clear(&vf);
			return false;
		}
		else
		{
			aPtr += ret;
			aNumBytes -= ret;
		}
	}
	ALuint aBuffer;
	alGenBuffers(1, &aBuffer);
	alBufferData(aBuffer, aFormat, aBuf, aLenBytes, anInfo->rate);

	mSoundBuffers[theSfxID] = aBuffer;

	delete[] aBuf;
	ov_clear(&vf);

	return true;
}

bool OpenALSoundManager::DecodeAUFormat(unsigned int theSfxID, const std::string& theFilename)
{
	PFILE* fp;

	fp = p_fopen((theFilename + ".au").c_str(), "rb");

	if (!fp)
		return false;

	char aHeaderId[5];
	aHeaderId[4] = '\0';
	p_fread(aHeaderId, 1, 4, fp);
	if (!strcmp(aHeaderId, ".snd") == 0)
		return false;

	ulong aHeaderSize;
	p_fread(&aHeaderSize, 4, 1, fp);
	aHeaderSize = LONG_BIGE_TO_NATIVE(aHeaderSize);

	ulong aDataSize;
	p_fread(&aDataSize, 4, 1, fp);
	aDataSize = LONG_BIGE_TO_NATIVE(aDataSize);

	ulong anEncoding;
	p_fread(&anEncoding, 4, 1, fp);
	anEncoding = LONG_BIGE_TO_NATIVE(anEncoding);

	ulong aSampleRate;
	p_fread(&aSampleRate, 4, 1, fp);
	aSampleRate = LONG_BIGE_TO_NATIVE(aSampleRate);

	ulong aChannelCount;
	p_fread(&aChannelCount, 4, 1, fp);
	aChannelCount = LONG_BIGE_TO_NATIVE(aChannelCount);

	p_fseek(fp, aHeaderSize, SEEK_SET);

	ulong aSrcBitCount = 8;
	ulong aBitCount = 16;
	switch (anEncoding)
	{
	case 1:
		aSrcBitCount = 8;
		aBitCount = 16;
		break;
	case 2:
		aSrcBitCount = 8;
		aBitCount = 8;
		break;


		case 3:
			aBitCount = 16;
			break;
        /* 		
		Support these formats?

		case 4:
			aBitCount = 24;
			break;
            
		case 5:
			aBitCount = 32;
			break;
        */

	default:
		return false;
	}

    std::vector<uint8_t> aSrcBuffer(aDataSize);
	int aReadSize = p_fread(aSrcBuffer.data(), 1, aDataSize, fp);
	p_fclose(fp);

	if (aReadSize != aDataSize)
		return false;

    std::vector<int16_t> decodedPCM;
	if (anEncoding == 1)
	{
		decodedPCM.resize(aDataSize);

		for (ulong i = 0; i < aDataSize; i++)
		{
			int ch = aSrcBuffer[i];

			int sign = (ch < 128) ? -1 : 1;
			ch = ch | 0x80;
			if (ch > 239)
				ch = ((0xF0 | 15) - ch) * 2;
			else if (ch > 223)
				ch = (((0xE0 | 15) - ch) * 4) + 32;
			else if (ch > 207)
				ch = (((0xD0 | 15) - ch) * 8) + 96;
			else if (ch > 191)
				ch = (((0xC0 | 15) - ch) * 16) + 224;
			else if (ch > 175)
				ch = (((0xB0 | 15) - ch) * 32) + 480;
			else if (ch > 159)
				ch = (((0xA0 | 15) - ch) * 64) + 992;
			else if (ch > 143)
				ch = (((0x90 | 15) - ch) * 128) + 2016;
			else if (ch > 128)
				ch = (((0x80 | 15) - ch) * 256) + 4064;
			else
				ch = 0xff;

			decodedPCM[i] = sign * ch * 4;
		}
	}
	else if (anEncoding == 2)
    {
        decodedPCM.resize(aDataSize);
        for (size_t i = 0; i < aDataSize; i++)
            decodedPCM[i] = (int16_t)((int)aSrcBuffer[i] - 128) << 8;
    }
    else if (anEncoding == 3)
    {
        decodedPCM.resize(aDataSize / 2);
        for (size_t i = 0; i < decodedPCM.size(); i++)
            decodedPCM[i] = (int16_t)((aSrcBuffer[i*2] << 8) | aSrcBuffer[i*2+1]);
    }
    else
        return false;
		

	ALenum aFormat = AL_NONE;
    if (aChannelCount == 1) 
        aFormat = AL_FORMAT_MONO16;
    else if (aChannelCount == 2) 
        aFormat = AL_FORMAT_STEREO16;
    else 
        return false;

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, aFormat, decodedPCM.data(), (ALsizei)(decodedPCM.size() * sizeof(int16_t)), aSampleRate);

    mSoundBuffers[theSfxID] = buffer;

	return true;
}