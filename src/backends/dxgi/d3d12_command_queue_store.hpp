#pragma once

#include "dxgi_win.hpp"

#include <set>
#include <atomic>

#include <utils/concurrency.hpp>

namespace gameoverlay::dxgi
{
    class d3d12_command_queue_store
    {
      public:
        void store(ID3D12CommandQueue& queue);
        CComPtr<ID3D12CommandQueue> find(const IDXGISwapChain3& swap_chain);

      private:
        std::atomic_bool active_{false};

        using queue_set = std::set<CComPtr<ID3D12CommandQueue>>;
        utils::concurrency::container<queue_set> queues_{};
    };
}
