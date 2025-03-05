#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"
#include <span>

namespace gameoverlay::dxgi
{
    class d3d12_canvas : public dxgi_canvas
    {
      public:
        d3d12_canvas(IDXGISwapChain3& swap_chain);
        d3d12_canvas(IDXGISwapChain3& swap_chain, dimensions dim);

        void paint(std::span<const uint8_t> image) override;
        void paint_i(std::span<const uint8_t> image) const;
        void draw() const override;

      private:
        CComPtr<IDXGISwapChain3> swap_chain_{};
        CComPtr<ID3D12Device> device_{};
        mutable CComPtr<ID3D12CommandQueue> command_queue_{};
        CComPtr<ID3D12GraphicsCommandList> command_list_{};
        CComPtr<ID3D12CommandAllocator> command_allocator_{};
        CComPtr<ID3D12DescriptorHeap> rtv_heap_{};
        CComPtr<ID3D12DescriptorHeap> srv_heap_{};
        // CComPtr<ID3D12Resource> render_targets_[2]{};
        CComPtr<ID3D12Resource> texture_{};
        CComPtr<ID3D12Resource> upload_buffer_{};
        CComPtr<ID3D12Resource> vertex_buffer_{};
        CComPtr<ID3D12Resource> index_buffer_{};
        CComPtr<ID3D12RootSignature> root_signature_{};
        CComPtr<ID3D12PipelineState> pipeline_state_{};
        D3D12_VIEWPORT viewport_{};
        D3D12_RECT scissor_rect_{};
        UINT rtv_descriptor_size_{};
        UINT srv_descriptor_size_{};
        HANDLE fence_event_{};
        CComPtr<ID3D12Fence> fence_{};
        mutable UINT64 fence_value_{};

        mutable std::vector<uint8_t> buffer{};

        void resize_texture(dimensions new_dimensions) override;
        void load_pipeline();
        void load_assets();
        void populate_command_list() const;
        void wait_for_gpu() const;
    };
}
