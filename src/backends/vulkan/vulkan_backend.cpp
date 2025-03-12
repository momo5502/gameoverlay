#include "vulkan_backend.hpp"

#include <backend_vulkan.hpp>
#include <utils/hook.hpp>

namespace gameoverlay::vulkan
{
    namespace
    {
        struct hooks
        {
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        // TODO: Synchronize access with object destruction
        vulkan_backend* g_backend{nullptr};

        void draw_frame(const HDC hdc)
        {
            if (!hdc || !g_backend)
            {
                return;
            }

            g_backend->create_or_access_renderer(
                hdc,
                [](vulkan_renderer& r) {
                    r.draw_frame(); //
                },
                hdc);
        }
    }

    vulkan_backend::~vulkan_backend()
    {
        g_backend = nullptr;
    }

    vulkan_backend::vulkan_backend(owned_handler h)
        : typed_backed(std::move(h))
    {
        g_backend = this;

        VkInstanceCreateInfo create_info = {};
        constexpr const char* instance_extension = "VK_KHR_surface";

        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = &instance_extension;

        VkInstance g_Instance{};
        vkCreateInstance(&create_info, nullptr, &g_Instance);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<vulkan_backend>(std::move(h));
    }
}
