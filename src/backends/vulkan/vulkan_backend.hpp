#pragma once

#include <backend.hpp>
#include "vulkan_renderer.hpp"

namespace gameoverlay::vulkan
{
    struct vulkan_backend : public typed_backed<backend_type::vulkan, vulkan_renderer, HDC>
    {
        vulkan_backend(owned_handler h);
        ~vulkan_backend() override;
    };
}
