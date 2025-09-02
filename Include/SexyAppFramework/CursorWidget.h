#pragma once

#include <SexyAppFramework/Widget.h>
#include <SexyAppFramework/Point.h>

namespace Sexy
{
	class Image;

	class CursorWidget : public Widget
	{
	public:
		Image *mImage;

	public:
		CursorWidget();

		virtual void Draw(Graphics *g);
		void SetImage(Image *theImage);
		Point GetHotspot();
	};
}
