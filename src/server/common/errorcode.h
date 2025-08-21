#pragma once

namespace ztofs 
{
enum ErrorCode 
{
    ZTO_OK = 0,
    ZTO_UNKNOWN_ERROR = -101,
    ZTO_FILE_NOT_FOUND = -201,
    ZTO_FILE_ALREADY_EXISTS = -202,
    ZTO_FILE_NOT_OPEN = -203,
    ZTO_CREATE_FAILED = -204,
    ZTO_OPEN_FAILED = -205,
};

} // namespace ztofs