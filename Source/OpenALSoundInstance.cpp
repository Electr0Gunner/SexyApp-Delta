#include <SexyAppFramework/OpenALSoundInstance.h>

#include <SexyAppFramework/OpenALSoundManager.h>
#include <cmath>

using namespace Sexy;

OpenALSoundInstance::OpenALSoundInstance(OpenALSoundManager* theSoundManager, ALuint theSourceSound)
{
	mSoundManagerP = theSoundManager;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;
	mSourceSoundBuffer = theSourceSound;
	mSoundSource = 0;

	mBaseVolume = 1.0;
	mBasePan = 0;

	mVolume = 1.0;
	mPan = 0;

	mDefaultFrequency = 44100;

	if (mSourceSoundBuffer != 0)
	{
		alGenSources(1, &mSoundSource);
        alSourcei(mSoundSource, AL_BUFFER, mSourceSoundBuffer);
		alGetBufferi(mSourceSoundBuffer, AL_FREQUENCY, &mDefaultFrequency);
	}

	RehupVolume();
}

OpenALSoundInstance::~OpenALSoundInstance()
{
	if (mSoundSource != NULL)
    {
        alSourceStop(mSoundSource);
        alSourcei(mSoundSource, AL_BUFFER, 0);
        alDeleteSources(1, &mSoundSource);
        mSoundSource = NULL;
    }
}

void OpenALSoundInstance::RehupVolume()
{
    if (mSoundSource != NULL)
		alSourcef(mSoundSource, AL_GAIN, mVolume * mBaseVolume * mSoundManagerP->mMasterVolume);
}

void OpenALSoundInstance::RehupPan()
{
    if (mSoundSource != NULL)
	{
		float aConvertedPanning = (mBasePan + mPan) / 10000.0f;
		if (aConvertedPanning < -1.0f)
			aConvertedPanning = -1.0f;
		if (aConvertedPanning > 1.0f)
			aConvertedPanning = 1.0f;

		alSource3f(mSoundSource, AL_POSITION, aConvertedPanning, 0.0f, 0.0f);
	}
}

void OpenALSoundInstance::Release()
{
	Stop();
	mReleased = true;
}

void OpenALSoundInstance::SetVolume(double theVolume) // 0 = max
{
	mVolume = theVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetPan(int thePosition) //-db to =db = left to right
{
	mPan = thePosition;
	RehupPan();
}

void OpenALSoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

bool OpenALSoundInstance::Play(bool looping, bool autoRelease)
{
	Stop();

	mHasPlayed = true;
	mAutoRelease = autoRelease;

	if (mSoundSource == NULL)
	{
		return false;
	}

	alSourcei(mSoundSource, AL_LOOPING, looping);
	alSourcePlay(mSoundSource);

	return true;
}

void OpenALSoundInstance::Stop()
{
	if (mSoundSource != NULL)
	{
		alSourceStop(mSoundSource);
        alSourcei(mSoundSource, AL_SAMPLE_OFFSET, 0);
        mAutoRelease = false;
	}
}

void OpenALSoundInstance::AdjustPitch(double theNumSteps)
{
	if (mSoundSource != NULL)
	{
        //1.0594630943592952645618252949463 is the twelved root of 2.
		double pitch = pow(1.0594630943592953, theNumSteps);

        double aMinPitch = 100.0 / mDefaultFrequency;
        double aMaxPitch = 100000.0 / mDefaultFrequency;

        if (pitch < aMinPitch) pitch = aMinPitch;
        if (pitch > aMaxPitch) pitch = aMaxPitch;

		alSourcef(mSoundSource, AL_PITCH, pitch);
	}
}

bool OpenALSoundInstance::IsPlaying()
{
	if (!mHasPlayed)
		return false;

	if (mSoundSource == NULL)
		return false;

	ALint aStatus;
	alGetSourcei(mSoundSource, AL_SOURCE_STATE, &aStatus);
	return aStatus == AL_PLAYING;
}

bool OpenALSoundInstance::IsReleased()
{
	if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))
		Release();

	return mReleased;
}

double OpenALSoundInstance::GetVolume()
{
	return mVolume;
}