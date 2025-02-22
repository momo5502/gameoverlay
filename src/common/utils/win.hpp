#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif
