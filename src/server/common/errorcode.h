#pragma once

namespace ztofs 
{
enum ErrorCode 
{
    ZTO_OK = 0,
    ZTO_UNKNOWN_ERROR = -101,
    ZTO_FILE_NOT_FOUND = -201,
    ZTO_FILE_ALREADY_EXISTS = -202,
};

} // namespace ztofs