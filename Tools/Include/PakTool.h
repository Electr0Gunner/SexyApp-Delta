#pragma once

#include <string>

namespace Sexy
{
    enum class PakErrorCode
    {
        ERROR_NONE = 0,
        ERROR_INVALID_MAGIC,
        ERROR_INVALID_VERSION,
    };
    class PakTool
    {
    public:
        PakTool() = default;

        static PakErrorCode Pack(const std::string& theInputPath, const std::string& thePakPath);
        static PakErrorCode Unpack(const std::string& thePakPath, const std::string& theOutputPath);
    };

} // namespace Sexy
