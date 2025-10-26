#pragma once

#include <SexyAppFramework/Common.h>
#include <windows.h>

namespace Sexy
{
	class SoundInstance;

#define MAX_SOURCE_SOUNDS 256
#define MAX_CHANNELS 32

	class SoundManager
	{
	public:
		SoundManager() {}
		virtual ~SoundManager() {}

		virtual bool Initialized() = nullptr;
		virtual void CollectGarbage() = 0;

		virtual bool LoadSound(unsigned int theSfxID, const std::string& theFilename) = nullptr;
		virtual int LoadSound(const std::string& theFilename) = nullptr;
		virtual void ReleaseSound(unsigned int theSfxID) = nullptr;

		virtual void SetVolume(double theVolume) = nullptr;
		virtual bool SetBaseVolume(unsigned int theSfxID, double theBaseVolume) = nullptr;
		virtual bool SetBasePan(unsigned int theSfxID, int theBasePan) = nullptr;

		virtual SoundInstance* GetSoundInstance(unsigned int theSfxID) = nullptr;

		virtual void ReleaseSounds() = nullptr;
		virtual void ReleaseChannels() = nullptr;

		virtual double GetMasterVolume() = nullptr;
		virtual void SetMasterVolume(double theVolume) = nullptr;

		virtual void Flush() = nullptr;
		virtual void SetCooperativeWindow(HWND theHWnd, bool isWindowed) = nullptr;
		virtual void StopAllSounds() = nullptr;
		virtual int GetFreeSoundId() = nullptr;
		virtual int GetNumSounds() = nullptr;
	};

} // namespace Sexy
