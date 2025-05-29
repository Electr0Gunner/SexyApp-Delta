#pragma warning(disable : 4786)

#include <SexyAppFramework/CritSect.h>
#include <windows.h>

using namespace Sexy;

////////////////////////////////////////////////////////////////////////////////

CritSect::CritSect(void)
{
	InitializeCriticalSection(&mCriticalSection);
}

////////////////////////////////////////////////////////////////////////////////

CritSect::~CritSect(void)
{
	DeleteCriticalSection(&mCriticalSection);
}
