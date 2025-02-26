#pragma once

#include "dxgi_win.hpp"

#include <dimensions.hpp>

namespace gameoverlay::dxgi
{
    struct d3d10_traits
    {
        using device = ID3D10Device;
        using context = ID3D10Device;
        using texture = ID3D10Texture2D;
        using buffer = ID3D10Buffer;

        using vertex_shader = ID3D10VertexShader;
        using pixel_shader = ID3D10PixelShader;
        using input_layout = ID3D10InputLayout;

        using blend_state = ID3D10BlendState;
        using sampler_state = ID3D10SamplerState;
        using rasterizer_state = ID3D10RasterizerState;
        using depth_stencil_state = ID3D10DepthStencilState;

        using render_target_view = ID3D10RenderTargetView;
        using shader_resource_view = ID3D10ShaderResourceView;

        using buffer_desc = D3D10_BUFFER_DESC;
        using input_element_desc = D3D10_INPUT_ELEMENT_DESC;
        using depth_stencil_desc = D3D10_DEPTH_STENCIL_DESC;
        using texture2d_desc = D3D10_TEXTURE2D_DESC;

        using subresource_data = D3D10_SUBRESOURCE_DATA;

        using viewport = D3D10_VIEWPORT;

        static constexpr auto INPUT_PER_VERTEX_DATA = D3D10_INPUT_PER_VERTEX_DATA;
        static constexpr auto USAGE_DEFAULT = D3D10_USAGE_DEFAULT;
        static constexpr auto USAGE_DYNAMIC = D3D10_USAGE_DYNAMIC;
        static constexpr auto BIND_INDEX_BUFFER = D3D10_BIND_INDEX_BUFFER;
        static constexpr auto BIND_VERTEX_BUFFER = D3D10_BIND_VERTEX_BUFFER;
        static constexpr auto CPU_ACCESS_WRITE = D3D10_CPU_ACCESS_WRITE;
        static constexpr auto DEPTH_WRITE_MASK_ALL = D3D10_DEPTH_WRITE_MASK_ALL;
        static constexpr auto COMPARISON_ALWAYS = D3D10_COMPARISON_ALWAYS;

        static constexpr auto PRIMITIVE_TOPOLOGY_TRIANGLELIST = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    struct d3d11_traits
    {
        using device = ID3D11Device;
        using context = ID3D11Device;
        using texture = ID3D11Texture2D;
        using buffer = ID3D11Buffer;

        using vertex_shader = ID3D11VertexShader;
        using pixel_shader = ID3D11PixelShader;
        using input_layout = ID3D11InputLayout;

        using blend_state = ID3D11BlendState;
        using sampler_state = ID3D11SamplerState;
        using rasterizer_state = ID3D11RasterizerState;
        using depth_stencil_state = ID3D11DepthStencilState;

        using render_target_view = ID3D11RenderTargetView;
        using shader_resource_view = ID3D11ShaderResourceView;

        using buffer_desc = D3D11_BUFFER_DESC;
        using input_element_desc = D3D11_INPUT_ELEMENT_DESC;
        using depth_stencil_desc = D3D11_DEPTH_STENCIL_DESC;
        using texture2d_desc = D3D11_TEXTURE2D_DESC;

        using subresource_data = D3D11_SUBRESOURCE_DATA;

        using viewport = D3D11_VIEWPORT;

        static constexpr auto INPUT_PER_VERTEX_DATA = D3D11_INPUT_PER_VERTEX_DATA;
        static constexpr auto USAGE_DEFAULT = D3D11_USAGE_DEFAULT;
        static constexpr auto USAGE_DYNAMIC = D3D11_USAGE_DYNAMIC;
        static constexpr auto BIND_INDEX_BUFFER = D3D11_BIND_INDEX_BUFFER;
        static constexpr auto BIND_VERTEX_BUFFER = D3D11_BIND_VERTEX_BUFFER;
        static constexpr auto CPU_ACCESS_WRITE = D3D11_CPU_ACCESS_WRITE;
        static constexpr auto DEPTH_WRITE_MASK_ALL = D3D11_DEPTH_WRITE_MASK_ALL;
        static constexpr auto COMPARISON_ALWAYS = D3D11_COMPARISON_ALWAYS;

        static constexpr auto PRIMITIVE_TOPOLOGY_TRIANGLELIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    template <typename Class>
    struct d3dx_trait_selector
    {
        static_assert(std::false_type::value, "Unsupported selection type");
    };

    template <>
    struct d3dx_trait_selector<ID3D10Device>
    {
        using traits = d3d10_traits;
    };

    template <>
    struct d3dx_trait_selector<ID3D11Device>
    {
        using traits = d3d11_traits;
    };

    template <>
    struct d3dx_trait_selector<ID3D11DeviceContext>
    {
        using traits = d3d11_traits;
    };

    template <typename Class>
    using d3dx_traits = typename d3dx_trait_selector<Class>::traits;

    template <typename Texture>
    struct texture_traits
    {
        static_assert(std::false_type::value, "Unsupported texture");
    };

    template <>
    struct texture_traits<ID3D10Texture2D>
    {
        using description = D3D10_TEXTURE2D_DESC;
    };

    template <>
    struct texture_traits<ID3D11Texture2D>
    {
        using description = D3D11_TEXTURE2D_DESC;
    };

    template <typename Device>
    struct device_traits
    {
        static_assert(std::false_type::value, "Unsupported device");
    };

    template <>
    struct device_traits<ID3D10Device>
    {
        using device = ID3D10Device;
        using context = ID3D10Device;
        using texture = ID3D10Texture2D;
    };

    template <>
    struct device_traits<ID3D11Device>
    {
        using device = ID3D11Device;
        using context = ID3D11DeviceContext;
        using texture = ID3D11Texture2D;
    };

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
        typename texture_traits<Texture>::description desc{};
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

        return get_texture_dimensions<Texture>(*back_buffer);
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
