#pragma once

#include <SexyAppFramework/Common.h>
#include <windows.h>

namespace Sexy
{
	class SoundInstance
	{
	public:
		SoundInstance() {}
		virtual ~SoundInstance() {}
		virtual void Release() = nullptr;

		virtual void SetBaseVolume(double theBaseVolume) = nullptr;
		virtual void SetBasePan(int theBasePan) = nullptr;

		virtual void AdjustPitch(double theNumSteps) = nullptr;

		virtual void SetVolume(double theVolume) = nullptr;
		virtual void SetPan(int thePosition) = nullptr; //-hundredth db to +hundredth db = left to right

		virtual bool Play(bool looping, bool autoRelease) = nullptr;
		virtual void Stop() = nullptr;
		virtual bool IsPlaying() = nullptr;
		virtual bool IsReleased() = nullptr;
		virtual double GetVolume() = nullptr;
	};
} // namespace Sexy
