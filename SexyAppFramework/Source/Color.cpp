#include <SexyAppFramework/Color.h>

using namespace Sexy;

Color Color::Black(0, 0, 0);
Color Color::White(255, 255, 255);

int	Color::GetRed() const
{
	return mRed;	
}

int Color::GetGreen() const
{
	return mGreen;
}

int	Color::GetBlue() const
{
	return mBlue;
}

int	Color::GetAlpha() const
{
	return mAlpha;
}

int& Color::operator[](int theIdx)
{
	static int aJunk = 0;

	switch (theIdx)
	{
	case 0:
		return mRed;
	case 1:
		return mGreen;
	case 2:
		return mBlue;
	case 3:
		return mAlpha;
	default:
		return aJunk;
	}
}

int Color::operator[](int theIdx) const
{
	switch (theIdx)
	{
	case 0:
		return mRed;
	case 1:
		return mGreen;
	case 2:
		return mBlue;
	case 3:
		return mAlpha;
	default:
		return 0;
	}
}

ulong Color::ToInt() const
{
	return (mAlpha << 24) | (mRed << 16) | (mGreen << 8) | (mBlue);
}

SexyRGBA Color::ToRGBA() const
{
	SexyRGBA anRGBA;
	anRGBA.r = mRed;
	anRGBA.g = mGreen;
	anRGBA.b = mBlue;
	anRGBA.a = mAlpha;

	return anRGBA;
}

bool Sexy::operator==(const Color& theColor1, const Color& theColor2)
{
	return 
		(theColor1.mRed == theColor2.mRed) &&
		(theColor1.mGreen == theColor2.mGreen) &&
		(theColor1.mBlue == theColor2.mBlue) && 
		(theColor1.mAlpha == theColor2.mAlpha);
}

bool Sexy::operator!=(const Color& theColor1, const Color& theColor2)
{
	return 
		(theColor1.mRed != theColor2.mRed) ||
		(theColor1.mGreen != theColor2.mGreen) ||
		(theColor1.mBlue != theColor2.mBlue) ||
		(theColor1.mAlpha != theColor2.mAlpha);
}
