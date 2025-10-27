#pragma once

#include <SexyAppFramework/Common.h>
#include <SexyAppFramework/CritSect.h>

namespace Sexy
{
	class AutoCrit
	{
	public:
		std::unique_lock<std::recursive_mutex> mLock;

	public:
			AutoCrit(std::recursive_mutex theCritSec) : mLock(theCritSec)
			{
			}

			AutoCrit(CritSect &theCritSect) : mLock(theCritSect.mCriticalSection)
			{
			}

			~AutoCrit()
			{
			}
	};
} // namespace Sexy
