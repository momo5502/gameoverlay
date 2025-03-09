#pragma once

#include <backend.hpp>
#include <memory>

#if defined(_WIN32) && !defined(_WIN64)
#define HAS_D3D8 1
#else
#define HAS_D3D8 0
#endif

namespace gameoverlay::d3d8
{
    std::unique_ptr<backend> create_backend(backend::owned_handler h);
}
