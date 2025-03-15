#include "vulkan_backend.hpp"

#include <backend_vulkan.hpp>
#include <utils/hook.hpp>

namespace gameoverlay::vulkan
{
    namespace
    {
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

        struct hooks
        {
            utils::hook::detour queue_present{};
            utils::hook::detour destroy_device{};
            utils::hook::detour acquire_next_image{};
            utils::hook::detour acquire_next_image_2{};
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

        void VKAPI_CALL destroy_device_stub(const VkDevice device, const VkAllocationCallbacks* allocator)
        {
            get_hooks().destroy_device.invoke_stdcall<void>(device, allocator);
        }

        VkResult VKAPI_CALL acquire_next_image_stub(const VkDevice device, const VkSwapchainKHR swapchain,
                                                    const uint64_t timeout, const VkSemaphore semaphore,
                                                    const VkFence fence, uint32_t* image_index)
        {
            return get_hooks().acquire_next_image.invoke_stdcall<VkResult>(device, swapchain, timeout, semaphore, fence,
                                                                           image_index);
        }

        VkResult VKAPI_CALL acquire_next_image_2_stub(const VkDevice device,
                                                      const VkAcquireNextImageInfoKHR* acquire_info,
                                                      uint32_t* image_index)
        {
            return get_hooks().acquire_next_image_2.invoke_stdcall<VkResult>(device, acquire_info, image_index);
        }

        VkResult VKAPI_CALL queue_present_stub(const VkQueue queue, const VkPresentInfoKHR* present_info)
        {
            return get_hooks().queue_present.invoke_stdcall<VkResult>(queue, present_info);
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

        auto* destroy_device = vkGetDeviceProcAddr(device, "vkDestroyDevice");
        hooks.destroy_device.create(destroy_device, &destroy_device_stub);

        auto* queue_present = vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
        hooks.queue_present.create(queue_present, &queue_present_stub);

        auto* acquire_next_image = vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
        hooks.acquire_next_image.create(acquire_next_image, &acquire_next_image_stub);

        auto* acquire_next_image_2 = vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHR");
        hooks.acquire_next_image_2.create(acquire_next_image_2, &acquire_next_image_2_stub);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<vulkan_backend>(std::move(h));
    }
}
