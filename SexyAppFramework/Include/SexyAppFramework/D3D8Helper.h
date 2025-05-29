#pragma once

#include <string>

namespace Sexy
{
	bool GetD3D8AdapterInfo(GUID& theGUID, std::string& theDriver, std::string& theDescription);
}
