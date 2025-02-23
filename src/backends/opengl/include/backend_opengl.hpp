#pragma once

#include <backend.hpp>
#include <memory>

namespace gameoverlay::opengl
{
    std::unique_ptr<backend> create_backend(backend::owned_handler h);
}
