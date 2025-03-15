#include "vulkan_backend.hpp"

#include <backend_vulkan.hpp>
#include <utils/hook.hpp>

namespace gameoverlay::vulkan
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour queue_present;
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

        VkResult VKAPI_CALL queue_present_stub(const VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
        {
            return get_hooks().queue_present.invoke_stdcall<VkResult>(queue, pPresentInfo);
        }

        utils::owned_object<VkInstance> create_vk_instance()
        {
            VkInstanceCreateInfo create_info = {};
            constexpr const char* instance_extension = "VK_KHR_surface";

            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.enabledExtensionCount = 1;
            create_info.ppEnabledExtensionNames = &instance_extension;

            VkInstance instance = VK_NULL_HANDLE;
            const auto res = vkCreateInstance(&create_info, nullptr, &instance);
            if (res != VK_SUCCESS)
            {
                return {};
            }

            return {instance, [](const VkInstance i) { vkDestroyInstance(i, nullptr); }};
        }

        VkPhysicalDevice get_gpu(const VkInstance instance)
        {
            uint32_t gpu_count{};
            vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);

            std::vector<VkPhysicalDevice> gpus(gpu_count);
            vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

            if (gpus.empty())
            {
                throw std::runtime_error("No gpus found");
            }

            for (auto& gpu : gpus)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(gpu, &properties);
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    return gpu;
                }
            }

            return gpus.front();
        }

        uint32_t get_queue_family_index(const VkPhysicalDevice gpu)
        {
            uint32_t count{};
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue_families.data());

            for (uint32_t i = 0; i < count; ++i)
            {
                if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    return i;
                }
            }

            return 0;
        }

        utils::owned_object<VkDevice> create_device(const VkPhysicalDevice gpu, const uint32_t queue_family_index)
        {
            constexpr float queue_priority = 1.0f;
            constexpr const char* device_extension = "VK_KHR_swapchain";

            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue_family_index;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &queue_priority;

            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = 1;
            create_info.pQueueCreateInfos = &queue_info;
            create_info.enabledExtensionCount = 1;
            create_info.ppEnabledExtensionNames = &device_extension;

            VkDevice device = VK_NULL_HANDLE;
            const auto res = vkCreateDevice(gpu, &create_info, nullptr, &device);
            if (res != VK_SUCCESS)
            {
                return {};
            }

            return {device, [](const VkDevice dev) { vkDestroyDevice(dev, nullptr); }};
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

        const auto instance = create_vk_instance();
        const auto gpu = get_gpu(instance);
        const auto queue_family_index = get_queue_family_index(gpu);
        const auto device = create_device(gpu, queue_family_index);

        auto& hooks = get_hooks();

        auto* queue_present = vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
        hooks.queue_present.create(queue_present, &queue_present_stub);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<vulkan_backend>(std::move(h));
    }
}
