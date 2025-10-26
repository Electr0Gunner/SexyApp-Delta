#include <SexyAppFramework/CursorWidget.h>
#include <SexyAppFramework/Image.h>

using namespace Sexy;

CursorWidget::CursorWidget()
{
	mImage = nullptr;
	mMouseVisible = false;
}

void CursorWidget::Draw(Graphics *g)
{
	if (mImage != nullptr)
		g->DrawImage(mImage, 0, 0);
}

void CursorWidget::SetImage(Image *theImage)
{
	mImage = theImage;
	if (mImage != nullptr)
		Resize(mX, mY, theImage->mWidth, theImage->mHeight);
}

Point CursorWidget::GetHotspot()
{
	if (mImage == nullptr)
		return Point(0, 0);
	return Point(mImage->GetWidth() / 2, mImage->GetHeight() / 2);
}
