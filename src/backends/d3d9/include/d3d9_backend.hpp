#pragma once

#include <backend.hpp>
#include <memory>

namespace gameoverlay::d3d9
{
    std::unique_ptr<backend> create_backend();
}
