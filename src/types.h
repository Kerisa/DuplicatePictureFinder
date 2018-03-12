#pragma once

#include <string>

namespace Alisa
{
#ifdef _UNICODE
    typedef std::wstring string_t;
#else
    typedef std::string string_t;
#endif
}