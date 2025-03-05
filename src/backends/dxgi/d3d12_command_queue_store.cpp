#include "d3d12_command_queue_store.hpp"

namespace gameoverlay::dxgi
{
    namespace
    {
        bool is_connected_queue(const IDXGISwapChain3& swap_chain, const ID3D12CommandQueue& queue)
        {
            constexpr size_t offset = 0x120;
            constexpr size_t entry = offset / sizeof(void*);
            constexpr size_t limit = entry + 20;

            const auto* values = reinterpret_cast<ID3D12CommandQueue* const*>(&swap_chain);

            for (size_t i = 0; i < limit; ++i)
            {
                if (values[i] == &queue)
                {
                    return true;
                }
            }

            return false;
        }
    }

    void d3d12_command_queue_store::store(ID3D12CommandQueue& queue)
    {
        if (!this->active_)
        {
            return;
        }

        this->queues_.access([&](queue_set& queues) {
            if (this->active_)
            {
                queues.insert(&queue); //
            }
        });
    }

    CComPtr<ID3D12CommandQueue> d3d12_command_queue_store::find(const IDXGISwapChain3& swap_chain)
    {
        this->active_ = true;

        return this->queues_.access<CComPtr<ID3D12CommandQueue>>([&](queue_set& queues) {
            CComPtr<ID3D12CommandQueue> target_queue{};

            for (auto& queue : queues)
            {
                if (is_connected_queue(swap_chain, *queue))
                {
                    target_queue = std::move(queue);
                    break;
                }
            }

            if (!target_queue)
            {
                return target_queue;
            }

            queues.clear();
            this->active_ = false;
            return target_queue;
        });
    }
}
