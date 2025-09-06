#pragma once

#include <SexyAppFramework/SoundManager.h>

#include <AL/al.h>
#include <AL/alc.h>

namespace Sexy
{
    class OpenALSoundManager : public SoundManager
    {
        friend class OpenALSoundInstance;


	protected:
		ALuint mSoundBuffers[MAX_SOURCE_SOUNDS];
		std::string mSourceFileNames[MAX_SOURCE_SOUNDS];
		ulong mSourceDataSizes[MAX_SOURCE_SOUNDS];
		double mBaseVolumes[MAX_SOURCE_SOUNDS];
		int mBasePans[MAX_SOURCE_SOUNDS];
		OpenALSoundInstance* mPlayingSounds[MAX_CHANNELS];
		double mMasterVolume;
		int mLastReleaseTick;

	protected:
		int FindFreeChannel();
		int VolumeToDB(double theVolume);
		bool DecodeOGGFormat(unsigned int theSfxID, const std::string& theFilename);
		bool DecodeAUFormat(unsigned int theSfxID, const std::string& theFilename);
		bool DecodeSound(unsigned int theSfxID, const std::string& theFilename);
		bool WriteWAV(unsigned int theSfxID, const std::string& theFilename, const std::string& theDepFile);
		bool GetTheFileTime(const std::string& theDepFile, FILETIME* theFileTime);
		void ReleaseFreeChannels();

	public:
		ALCdevice* mSoundDevice;
		ALCcontext* mSoundContext;

		OpenALSoundManager();
		virtual ~OpenALSoundManager();

		virtual bool Initialized();

		virtual bool LoadSound(unsigned int theSfxID, const std::string& theFilename);
		virtual int LoadSound(const std::string& theFilename);
		virtual void ReleaseSound(unsigned int theSfxID);

		virtual void SetVolume(double theVolume);
		virtual bool SetBaseVolume(unsigned int theSfxID, double theBaseVolume);
		virtual bool SetBasePan(unsigned int theSfxID, int theBasePan);

		virtual SoundInstance* GetSoundInstance(unsigned int theSfxID);

		virtual void ReleaseSounds();
		virtual void ReleaseChannels();

		virtual double GetMasterVolume();
		virtual void SetMasterVolume(double theVolume);

		virtual void Flush();

		virtual void SetCooperativeWindow(HWND theHWnd, bool isWindowed);
		virtual void StopAllSounds();
		virtual int GetFreeSoundId();
		virtual int GetNumSounds();
    };
    
} // namespace Sexy
