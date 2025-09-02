#pragma once

#include <SexyAppFramework/Common.h>

namespace Sexy
{
#pragma pack(push, 1)
	struct SexyRGBA
	{
		unsigned char b, g, r, a;
	};
#pragma pack(pop)

	class Color
	{
	public:
		int mRed;
		int mGreen;
		int mBlue;
		int mAlpha;

		static Color Black;
		static Color White;

	public:
		constexpr Color() :
			mRed(0),
			mGreen(0),
			mBlue(0),
			mAlpha(255)
		{
		}

		constexpr Color(int theColor) :
			mAlpha((theColor >> 24) & 0xFF),
			mRed((theColor >> 16) & 0xFF),
			mGreen((theColor >> 8) & 0xFF),
			mBlue((theColor) & 0xFF)
		{
			if (mAlpha == 0)
				mAlpha = 0xff;
		}

		constexpr Color(int theColor, int theAlpha) :
			mRed((theColor >> 16) & 0xFF),
			mGreen((theColor >> 8) & 0xFF),
			mBlue((theColor) & 0xFF),
			mAlpha(theAlpha)
		{
		}

		constexpr Color(int theRed, int theGreen, int theBlue) :
			mRed(theRed),
			mGreen(theGreen),
			mBlue(theBlue),
			mAlpha(0xFF)
		{
		}

		constexpr Color(int theRed, int theGreen, int theBlue, int theAlpha) :
			mRed(theRed),
			mGreen(theGreen),
			mBlue(theBlue),
			mAlpha(theAlpha)
		{
		}

		constexpr Color(const SexyRGBA& theColor) :
			mRed(theColor.r),
			mGreen(theColor.g),
			mBlue(theColor.b),
			mAlpha(theColor.a)
		{
		}

		constexpr Color(const uchar* theElements) :
			mRed(theElements[0]),
			mGreen(theElements[1]),
			mBlue(theElements[2]),
			mAlpha(0xFF)
		{
		}

		constexpr Color(const int* theElements) :
			mRed(theElements[0]),
			mGreen(theElements[1]),
			mBlue(theElements[2]),
			mAlpha(0xFF)
		{
		}

		int GetRed() const;
		int GetGreen() const;
		int GetBlue() const;
		int GetAlpha() const;
		ulong ToInt() const;
		SexyRGBA ToRGBA() const;

		int& operator[](int theIdx);
		int operator[](int theIdx) const;
	};

	bool operator==(const Color& theColor1, const Color& theColor2);
	bool operator!=(const Color& theColor1, const Color& theColor2);

} // namespace Sexy
