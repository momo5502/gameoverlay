#pragma once

#include <backend.hpp>
#include "vulkan_renderer.hpp"

namespace gameoverlay::vulkan
{
    struct vulkan_backend : typed_backed<vulkan_renderer, VkSwapchainKHR>
    {
        vulkan_backend(owned_handler h);
        ~vulkan_backend() override;
    };
}
