#pragma once

#include "SoundInstance.h"

#include <AL/al.h>

namespace Sexy
{
    class OpenALSoundInstance : public SoundInstance
    {
		friend class OpenALSoundManager;

	protected:
		OpenALSoundManager* mSoundManagerP;
		ALuint mSourceSoundBuffer;
		ALuint mSoundSource;
		bool mAutoRelease;
		bool mHasPlayed;
		bool mReleased;

		int mBasePan;
		double mBaseVolume;

		int mPan;
		double mVolume;

		int mDefaultFrequency;

	protected:
		void RehupVolume();
		void RehupPan();

	public:
		OpenALSoundInstance(OpenALSoundManager* theSoundManager, ALuint theSourceSound);
		virtual ~OpenALSoundInstance();
		virtual void Release();

		virtual void SetBaseVolume(double theBaseVolume);
		virtual void SetBasePan(int theBasePan);

		virtual void SetVolume(double theVolume);
		virtual void SetPan(int thePosition); //-hundredth db to +hundredth db = left to right
		virtual void AdjustPitch(double theNumSteps);

		virtual bool Play(bool looping, bool autoRelease);
		virtual void Stop();
		virtual bool IsPlaying();
		virtual bool IsReleased();
		virtual double GetVolume();
    };

    
} // namespace Sexy
