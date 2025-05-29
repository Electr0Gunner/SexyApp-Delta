#pragma once

#include <SexyAppFramework/Common.h>

namespace Sexy
{
	bool Quantize8Bit(const ulong* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, ulong* theDestColorTable);
}
