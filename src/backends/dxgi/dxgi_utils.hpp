#pragma once

#include "dxgi_traits.hpp"

#include <optional>
#include <dimensions.hpp>

namespace gameoverlay::dxgi
{
    template <typename Device>
    CComPtr<Device> get_device(IDXGISwapChain& swap_chain)
    {
        CComPtr<Device> device{};
        if (FAILED(swap_chain.GetDevice(__uuidof(Device), reinterpret_cast<void**>(&device))))
        {
            return {};
        }

        return device;
    }

    inline CComPtr<ID3D10Device> get_context(ID3D10Device& device)
    {
        return {&device};
    }

    inline CComPtr<ID3D11DeviceContext> get_context(ID3D11Device& device)
    {
        CComPtr<ID3D11DeviceContext> context{};
        device.GetImmediateContext(&context);
        return context;
    }

    template <typename Texture>
    CComPtr<Texture> get_back_buffer(IDXGISwapChain& swap_chain)
    {
        CComPtr<Texture> back_buffer{};
        const auto res = swap_chain.GetBuffer(0, __uuidof(Texture), reinterpret_cast<void**>(&back_buffer));
        if (FAILED(res) || !back_buffer)
        {
            return {};
        }

        return back_buffer;
    }

    template <typename Texture>
    dimensions get_texture_dimensions(Texture& texture)
    {
        typename d3dx_traits<Texture>::texture2d_desc desc{};
        texture.GetDesc(&desc);

        return {
            desc.Width,
            desc.Height,
        };
    }

    template <typename Texture>
    dimensions get_swap_chain_dimensions(IDXGISwapChain& swap_chain)
    {
        auto back_buffer = get_back_buffer<Texture>(swap_chain);
        if (!back_buffer)
        {
            return {};
        }

        return get_texture_dimensions(*back_buffer);
    }

    inline std::optional<DXGI_SWAP_CHAIN_DESC> get_swapchain_desc(IDXGISwapChain& swap_chain)
    {
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swap_chain.GetDesc(&desc)))
        {
            return std::nullopt;
        }

        return desc;
    }

    inline HWND get_swap_chain_window(IDXGISwapChain& swap_chain)
    {
        const auto desc = get_swapchain_desc(swap_chain);

        if (!desc)
        {
            return {};
        }

        return desc->OutputWindow;
    }

    inline bool is_srgb_format(const DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return true;
        default:
            return false;
        }
    }
}
