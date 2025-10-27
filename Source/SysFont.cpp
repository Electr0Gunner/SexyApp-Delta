#include <SexyAppFramework/SysFont.h>

#include <SexyAppFramework/Renderer.h>
#include <SexyAppFramework/GPUImage.h>
#include <SexyAppFramework/Graphics.h>
#include <SexyAppFramework/ImageFont.h>
#include <SexyAppFramework/MemoryImage.h>
#include <SexyAppFramework/SexyAppBase.h>
#include <SexyAppFramework/WidgetManager.h>
#include <stdlib.h>

using namespace Sexy;

SysFont::SysFont(const std::string &theFace, int thePointSize, bool bold, bool italics, bool underline)
{
	Init(gSexyAppBase, theFace, thePointSize, 0, bold, italics, underline, false);
}

SysFont::SysFont(SexyAppBase *theApp, const std::string &theFace, int thePointSize, int theScript, bool bold, bool italics,
				 bool underline)
{
	Init(theApp, theFace, thePointSize, theScript, bold, italics, underline, true);
}

SysFont::SysFont(SexyAppBase *theApp, const unsigned char aData[], size_t aDataSize, int thePointSize, int theScript,
				 bool bold, bool italics, bool underline)
{
	mApp = theApp;
	SDL_IOStream *io = SDL_IOFromConstMem((void *)aData, aDataSize);
	if (!io)
	{
		SDL_ShowSimpleMessageBox(static_cast<SDL_MessageBoxFlags>(MsgBox_OK), "Failed to create SDL_IOStream", SDL_GetError(), mApp->mWindow);
		return;
	}

	mFont = TTF_OpenFontIO(io, false, thePointSize);
	if (!mFont)
	{
		SDL_ShowSimpleMessageBox(static_cast<SDL_MessageBoxFlags>(MsgBox_OK), "Error", SDL_GetError(), mApp->mWindow);
	}

	TTF_SetFontStyle(mFont, (bold ? TTF_STYLE_BOLD : 0) | (italics ? TTF_STYLE_ITALIC : 0) |
								   (underline ? TTF_STYLE_UNDERLINE : 0));

	mAscent = TTF_GetFontAscent(mFont);
	mHeight = TTF_GetFontHeight(mFont);

	mDrawShadow = false;
	mSimulateBold = false;
}

void SysFont::Init(SexyAppBase *theApp, const std::string &theFace, int thePointSize, int theScript, bool bold,
				   bool italics, bool underline, bool useDevCaps)
{
	mApp = theApp;

	mFont = TTF_OpenFont(theFace.c_str(), thePointSize);
	if (!mFont)
	{
		SDL_ShowSimpleMessageBox(static_cast<SDL_MessageBoxFlags>(MsgBox_OK), "Error", SDL_GetError(), mApp->mWindow);
	}

	TTF_SetFontStyle(mFont, (bold ? TTF_STYLE_BOLD : 0) | (italics ? TTF_STYLE_ITALIC : 0) |
								   (underline ? TTF_STYLE_UNDERLINE : 0));

	mAscent = TTF_GetFontAscent(mFont);
	mHeight = TTF_GetFontHeight(mFont);

	mDrawShadow = false;
	mSimulateBold = false;
}

SysFont::SysFont(const SysFont &theSysFont)
{
	mFont = theSysFont.mFont;
	mApp = theSysFont.mApp;
	mHeight = theSysFont.mHeight;
	mAscent = theSysFont.mAscent;

	mDrawShadow = false;
	mSimulateBold = false;
}

SysFont::~SysFont()
{
	TTF_CloseFont(mFont);
}

ImageFont* SysFont::CreateImageFont()
{
	/*
	int i;
	MemoryImage* anImage;
	int anImageCharWidth, anImageXOff, anImageYOff;

	////////////////////////////////////////////////////
	// Step 1: Create image
	anImageCharWidth = CharWidth('W') * 2;
	anImageXOff = anImageCharWidth / 4;
	anImageYOff = mHeight / 2;
	int aWidth = 257 * anImageCharWidth;
	int aHeight = mHeight * 2;

	BITMAPINFO aBitmapInfo;
	memset(&aBitmapInfo, 0, sizeof(aBitmapInfo));
	BITMAPINFOHEADER& aHeader = aBitmapInfo.bmiHeader;
	aHeader.biSize = sizeof(aHeader);
	aHeader.biWidth = aWidth;
	aHeader.biHeight = -aHeight;
	aHeader.biPlanes = 1;
	aHeader.biBitCount = 32;
	aHeader.biCompression = BI_RGB;

	HDC aDC = CreateCompatibleDC(nullptr);

	DWORD* aBits = nullptr;
	HBITMAP aBitmap = CreateDIBSection(aDC, &aBitmapInfo, DIB_RGB_COLORS, (void**)&aBits, nullptr, 0);

	HBITMAP anOldBitmap = (HBITMAP)SelectObject(aDC, aBitmap);
	HFONT anOldFont = (HFONT)SelectObject(aDC, mHFont);

	HBRUSH anOldBrush = (HBRUSH)SelectObject(aDC, GetStockObject(BLACK_BRUSH));
	Rectangle(aDC, 0, 0, aWidth, aHeight);

	SetBkMode(aDC, TRANSPARENT);
	SetTextColor(aDC, RGB(255, 255, 255));

	int xpos = anImageXOff;
	int ypos = anImageYOff;
	for (i = 0; i < 256; i++)
	{
		char aChar = i;
		TextOutA(aDC, xpos, ypos, &aChar, 1);
		xpos += anImageCharWidth;
	}
	GdiFlush();

	SelectObject(aDC, anOldBrush);
	SelectObject(aDC, anOldBitmap);
	SelectObject(aDC, anOldFont);

	int aSize = aWidth * aHeight;
	anImage = new MemoryImage(mApp);
	anImage->Create(aWidth, aHeight);
	DWORD* src = aBits;
	DWORD* dst = anImage->GetBits();
	for (i = 0; i < aSize; i++)
	{
		DWORD anAlpha = ((*src++) & 0xff) << 24;
		*dst++ = anAlpha | 0xFFFFFF;
	}
	anImage->BitsChanged();
	DeleteObject(aBitmap);

	////////////////////////////////////////////////////
	// Step 2: Create image font

	ImageFont* aFont = new ImageFont(anImage);
	FontLayer* aFontLayer = &aFont->mFontData->mFontLayerList.back();

	aFontLayer->mAscent = mAscent;
	aFontLayer->mHeight = mHeight;

	for (i = 0; i < 256; i++)
	{
		char aChar = i;

		aFontLayer->mCharData[(uchar)aChar].mImageRect = Rect(aChar * anImageCharWidth, 0, anImageCharWidth, anImage->mHeight);
		aFontLayer->mCharData[(uchar)aChar].mWidth = CharWidth(aChar);
		aFontLayer->mCharData[(uchar)aChar].mOffset = Point(-anImageXOff, -anImageYOff);
	}

	aFont->GenerateActiveFontLayers();
	aFont->mActiveListValid = true;
*/
	return nullptr; //TODO: implement
}

int SysFont::StringWidth(const SexyString& theString)
{
	int aWidth = 0;
	TTF_GetStringSize(mFont, theString.c_str(), 0, &aWidth, nullptr);
	return aWidth;
}

void SysFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect)
{
	SDL_Color aColor = {theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha};

	SDL_Surface* aTextSurface = TTF_RenderText_Blended(mFont, theString.c_str(), 0, aColor);

	MemoryImage aTempImage;
	aTempImage.Create(aTextSurface->w, aTextSurface->h);
	memcpy(aTempImage.GetBits(), aTextSurface->pixels, aTextSurface->w * aTextSurface->h * sizeof(decltype(aTempImage.mBits))); //Save way to get the size of the type defining mBits, useful for when we change it to uint32_t later

	g->PushState();
	g->mClipRect = g->mClipRect.Intersection(theClipRect);
	g->DrawImageF(&aTempImage, theX, theY);
	g->PopState();
}

Font* SysFont::Duplicate()
{
	return new SysFont(*this);
}
