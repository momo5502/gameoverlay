#pragma once

#include <backend.hpp>
#include <memory>

namespace gameoverlay::d3d8
{
    std::unique_ptr<backend> create_backend(backend::owned_handler h);
}
