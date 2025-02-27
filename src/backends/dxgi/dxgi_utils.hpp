#pragma once

#include "dxgi_traits.hpp"

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

    inline HWND get_swap_chain_window(IDXGISwapChain& swap_chain)
    {
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swap_chain.GetDesc(&desc)))
        {
            return {};
        }

        return desc.OutputWindow;
    }
}
