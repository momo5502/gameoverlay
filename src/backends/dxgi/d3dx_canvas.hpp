#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"
#include "dxgi_canvas_utils.hpp"

#include <array>
#include <stdexcept>
#include <string_view>

#include <utils/finally.hpp>

namespace gameoverlay::dxgi
{
    namespace detail
    {
        template <typename Context>
        struct context_store
        {
            using traits = d3dx_traits<Context>;

            UINT ref{};
            CComPtr<Context> context{};
            CComPtr<typename traits::depth_stencil_state> depth_stencil_state{};
            CComPtr<typename traits::depth_stencil_view> depth_stencil_view{};

            static_assert(sizeof(CComPtr<typename traits::render_target_view>) ==
                          sizeof(typename traits::render_target_view));

            std::array<CComPtr<typename traits::render_target_view>, traits::SIMULTANEOUS_RENDER_TARGET_COUNT>
                render_target_views{};

            context_store(Context& c)
                : context(&c)
            {
                c.OMGetRenderTargets(static_cast<uint32_t>(this->render_target_views.size()),
                                     &this->render_target_views[0], &this->depth_stencil_view);
                c.OMGetDepthStencilState(&this->depth_stencil_state, &this->ref);
            }

            ~context_store()
            {
                auto& c = *this->context;

                c.OMSetDepthStencilState(this->depth_stencil_state, this->ref);
                c.OMSetRenderTargets(static_cast<uint32_t>(this->render_target_views.size()),
                                     &this->render_target_views[0].p, this->depth_stencil_view);
            }
        };

        inline CComPtr<ID3D10VertexShader> compile_vertex_shader(ID3D10Device& device, ID3DBlob** blob)
        {
            CComPtr<ID3D10VertexShader> shader{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3DCompile(vertex_shader_src.data(), vertex_shader_src.size(), nullptr, nullptr, nullptr, "VS",
                                  "vs_4_0", 0, 0, blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile vertex shader");
            }

            res = device.CreateVertexShader((*blob)->GetBufferPointer(), (*blob)->GetBufferSize(), &shader);
            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create vertex shader");
            }

            return shader;
        }

        inline CComPtr<ID3D11VertexShader> compile_vertex_shader(ID3D11Device& device, ID3DBlob** blob)
        {
            CComPtr<ID3D11VertexShader> shader{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3DCompile(vertex_shader_src.data(), vertex_shader_src.size(), nullptr, nullptr, nullptr, "VS",
                                  "vs_5_0", 0, 0, blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile vertex shader");
            }

            res = device.CreateVertexShader((*blob)->GetBufferPointer(), (*blob)->GetBufferSize(), nullptr, &shader);
            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create vertex shader");
            }

            return shader;
        }

        inline CComPtr<ID3D10PixelShader> compile_pixel_shader(ID3D10Device& device)
        {
            CComPtr<ID3D10PixelShader> shader{};
            CComPtr<ID3DBlob> shader_blob{};
            CComPtr<ID3DBlob> error_blob{};

            auto res = D3DCompile(pixel_shader_src.data(), pixel_shader_src.size(), nullptr, nullptr, nullptr, "PS",
                                  "ps_4_0", 0, 0, &shader_blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile pixel shader");
            }

            res = device.CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), &shader);
            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create pixel shader");
            }

            return shader;
        }

        inline CComPtr<ID3D11PixelShader> compile_pixel_shader(ID3D11Device& device)
        {
            CComPtr<ID3D11PixelShader> shader{};
            CComPtr<ID3DBlob> shader_blob{};
            CComPtr<ID3DBlob> error_blob{};

            auto res = D3DCompile(pixel_shader_src.data(), pixel_shader_src.size(), nullptr, nullptr, nullptr, "PS",
                                  "ps_5_0", 0, 0, &shader_blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile pixel shader");
            }

            res = device.CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr,
                                           &shader);
            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create pixel shader");
            }

            return shader;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::input_layout> create_input_layout(Device& device, ID3DBlob& vs_blob)
        {
            using traits = d3dx_traits<Device>;

            typename traits::input_element_desc layout[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, traits::INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, traits::INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, traits::INPUT_PER_VERTEX_DATA, 0},
            };

            CComPtr<typename traits::input_layout> input_layout{};
            const auto res = device.CreateInputLayout(layout, ARRAYSIZE(layout), vs_blob.GetBufferPointer(),
                                                      vs_blob.GetBufferSize(), &input_layout);
            if (FAILED(res) || !input_layout)
            {
                throw std::runtime_error("Failed to create input layout");
            }

            return input_layout;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::buffer> create_buffer(
            Device& device,                                               //
            const typename d3dx_traits<Device>::buffer_desc& buffer_desc, //
            const typename d3dx_traits<Device>::subresource_data* init_data = nullptr)
        {
            CComPtr<typename d3dx_traits<Device>::buffer> buffer{};
            const auto res = device.CreateBuffer(&buffer_desc, init_data, &buffer);

            if (FAILED(res) || !buffer)
            {
                return {};
            }

            return buffer;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::buffer> create_index_buffer(Device& device)
        {
            using traits = d3dx_traits<Device>;

            const DWORD indices[] = {
                0, 1, 2, 0, 2, 3,
            };

            typename traits::buffer_desc index_buffer_desc{};
            index_buffer_desc.Usage = traits::USAGE_DEFAULT;
            index_buffer_desc.ByteWidth = sizeof(DWORD) * 2 * 3;
            index_buffer_desc.BindFlags = traits::BIND_INDEX_BUFFER;
            index_buffer_desc.CPUAccessFlags = 0;
            index_buffer_desc.MiscFlags = 0;

            typename traits::subresource_data iinit_data;
            iinit_data.pSysMem = indices;

            auto index_buffer = create_buffer(device, index_buffer_desc, &iinit_data);

            if (!index_buffer)
            {
                throw std::runtime_error("Failed to create index buffer");
            }

            return index_buffer;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::buffer> create_vertex_buffer(Device& device)
        {
            using traits = d3dx_traits<Device>;

            typename traits::buffer_desc vertex_buffer_desc{};
            vertex_buffer_desc.Usage = traits::USAGE_DYNAMIC;
            vertex_buffer_desc.ByteWidth = sizeof(vertex) * 4;
            vertex_buffer_desc.BindFlags = traits::BIND_VERTEX_BUFFER;
            vertex_buffer_desc.CPUAccessFlags = traits::CPU_ACCESS_WRITE;

            auto index_buffer = create_buffer(device, vertex_buffer_desc);
            if (!index_buffer)
            {
                throw std::runtime_error("Failed to create index buffer");
            }

            return index_buffer;
        }

        inline void fill_bend_desc(D3D10_BLEND_DESC& blend_desc)
        {
            blend_desc.BlendEnable[0] = TRUE;
            blend_desc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
            blend_desc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
            blend_desc.BlendOp = D3D10_BLEND_OP_ADD;
            blend_desc.SrcBlendAlpha = D3D10_BLEND_ZERO;
            blend_desc.DestBlendAlpha = D3D10_BLEND_ZERO;
            blend_desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
            blend_desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
        }

        inline void fill_bend_desc(D3D11_BLEND_DESC& blend_desc)
        {
            blend_desc.RenderTarget[0].BlendEnable = TRUE;
            blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
            blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::blend_state> create_blend_state(Device& device)
        {
            using traits = d3dx_traits<Device>;

            typename traits::blend_desc blend_desc{};
            fill_bend_desc(blend_desc);

            CComPtr<typename traits::blend_state> blend_state{};
            const auto res = device.CreateBlendState(&blend_desc, &blend_state);

            if (FAILED(res) || !blend_state)
            {
                throw std::runtime_error("Failed to create blend state");
            }

            return blend_state;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::depth_stencil_state> create_depth_stencil_state(Device& device)
        {
            using traits = d3dx_traits<Device>;

            typename traits::depth_stencil_desc depth_stencil_desc{};
            depth_stencil_desc.DepthEnable = false;
            depth_stencil_desc.DepthWriteMask = traits::DEPTH_WRITE_MASK_ALL;
            depth_stencil_desc.DepthFunc = traits::COMPARISON_ALWAYS;
            depth_stencil_desc.StencilEnable = false;

            CComPtr<typename traits::depth_stencil_state> depth_stencil_state{};
            const auto res = device.CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);

            if (FAILED(res) || !depth_stencil_state)
            {
                throw std::runtime_error("Failed to create depth stencil state");
            }

            return depth_stencil_state;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::texture2d> create_texture_2d(Device& device, const dimensions dim,
                                                                           const DXGI_FORMAT format)
        {
            using traits = d3dx_traits<Device>;

            typename traits::texture2d_desc desc{};
            desc.Width = dim.width;
            desc.Height = dim.height;
            desc.MipLevels = desc.ArraySize = 1;
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.Usage = traits::USAGE_DYNAMIC;
            desc.BindFlags = traits::BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = traits::CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            CComPtr<typename traits::texture2d> texture{};
            const auto res = device.CreateTexture2D(&desc, nullptr, &texture);

            if (FAILED(res) || !texture)
            {
                throw std::runtime_error("Failed to create texture");
            }

            return texture;
        }

        template <typename Texture>
        CComPtr<typename d3dx_traits<Texture>::shader_resource_view> create_shader_resource_view(Texture& texture)
        {
            using traits = d3dx_traits<Texture>;

            CComPtr<typename traits::device> device{};
            texture.GetDevice(&device);

            typename traits::texture2d_desc desc{};
            texture.GetDesc(&desc);

            typename traits::shader_resource_view_desc srv_desc{};
            srv_desc.Format = desc.Format;
            srv_desc.ViewDimension = traits::SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MipLevels = 1;
            srv_desc.Texture2D.MostDetailedMip = 0;

            CComPtr<typename traits::shader_resource_view> shader_resource_view{};
            const auto res = device->CreateShaderResourceView(&texture, &srv_desc, &shader_resource_view);

            if (FAILED(res) || !shader_resource_view)
            {
                throw std::runtime_error("Failed to create shader resource view");
            }

            return shader_resource_view;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::rasterizer_state> create_rasterizer_state(Device& device)
        {
            using traits = d3dx_traits<Device>;

            typename traits::rasterizer_desc rasterizer_desc{};
            rasterizer_desc.FillMode = traits::FILL_SOLID;
            rasterizer_desc.CullMode = traits::CULL_NONE;
            rasterizer_desc.FrontCounterClockwise = TRUE;

            CComPtr<typename traits::rasterizer_state> rasterizer_state{};
            const auto res = device.CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

            if (FAILED(res) || !rasterizer_state)
            {
                throw std::runtime_error("Failed to create rasterizer state");
            }

            return rasterizer_state;
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::render_target_view> create_render_target_view(Device& device,
                                                                                            IDXGISwapChain& swap_chain)
        {
            using traits = d3dx_traits<Device>;

            const auto back_buffer = get_back_buffer<typename traits::texture2d>(swap_chain);
            if (!back_buffer)
            {
                throw std::runtime_error("Failed to get back buffer");
            }

            CComPtr<typename traits::render_target_view> render_target_view{};
            const auto res = device.CreateRenderTargetView(back_buffer, nullptr, &render_target_view);

            if (FAILED(res) || !render_target_view)
            {
                throw std::runtime_error("Failed to create render target view");
            }

            return render_target_view;
        }

        template <typename traits>
        bool is_srgb(IDXGISwapChain& swap_chain)
        {
            const auto back_buffer = get_back_buffer<typename traits::texture2d>(swap_chain);
            if (!back_buffer)
            {
                return false;
            }

            typename traits::texture2d_desc desc{};
            back_buffer->GetDesc(&desc);

            return is_srgb_format(desc.Format);
        }

        template <typename Device>
        CComPtr<typename d3dx_traits<Device>::sampler_state> create_sampler_state(Device& device)
        {
            using traits = d3dx_traits<Device>;

            typename traits::sampler_desc sampler_desc{};
            sampler_desc.Filter = traits::FILTER_MIN_MAG_MIP_LINEAR;
            sampler_desc.AddressU = traits::TEXTURE_ADDRESS_CLAMP;
            sampler_desc.AddressV = traits::TEXTURE_ADDRESS_CLAMP;
            sampler_desc.AddressW = traits::TEXTURE_ADDRESS_CLAMP;
            sampler_desc.ComparisonFunc = traits::COMPARISON_NEVER;
            sampler_desc.MinLOD = 0;
            sampler_desc.MaxLOD = traits::FLOAT32_MAX;

            CComPtr<typename traits::sampler_state> sampler_state{};
            const auto res = device.CreateSamplerState(&sampler_desc, &sampler_state);

            if (FAILED(res) || !sampler_state)
            {
                throw std::runtime_error("Failed to create sampler state");
            }

            return sampler_state;
        }

        template <typename Accessor>
        void access_buffer(ID3D10Buffer& buffer, const Accessor& accessor)
        {
            void* mapped_data{};
            if (FAILED(buffer.Map(D3D10_MAP_WRITE_DISCARD, 0, &mapped_data)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                buffer.Unmap(); //
            });

            accessor(mapped_data);
        }

        template <typename Accessor>
        void access_buffer(ID3D11Buffer& buffer, const Accessor& accessor)
        {
            CComPtr<ID3D11Device> device{};
            buffer.GetDevice(&device);

            CComPtr<ID3D11DeviceContext> context{};
            device->GetImmediateContext(&context);

            D3D11_MAPPED_SUBRESOURCE mapped_data{};
            if (FAILED(context->Map(&buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                context->Unmap(&buffer, 0); //
            });
            accessor(mapped_data.pData);
        }

        template <typename Accessor>
        void access_texture(ID3D10Texture2D& texture, const Accessor& accessor)
        {
            D3D10_MAPPED_TEXTURE2D texmap{};
            if (FAILED(texture.Map(0, D3D10_MAP_WRITE_DISCARD, 0, &texmap)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                texture.Unmap(0); //
            });

            accessor(texmap.pData, texmap.RowPitch);
        }

        template <typename Accessor>
        void access_texture(ID3D11Texture2D& texture, const Accessor& accessor)
        {
            CComPtr<ID3D11Device> device{};
            texture.GetDevice(&device);

            CComPtr<ID3D11DeviceContext> context{};
            device->GetImmediateContext(&context);

            D3D11_MAPPED_SUBRESOURCE texmap{};
            if (FAILED(context->Map(&texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                context->Unmap(&texture, 0); //
            });

            accessor(texmap.pData, texmap.RowPitch);
        }

        template <typename Buffer>
        void translate_vertices(Buffer& vertex_buffer, const int32_t x, const int32_t y, const COLORREF color,
                                const dimensions dim)
        {
            access_buffer(vertex_buffer, [&](void* data) {
                auto* v = static_cast<vertex*>(data);
                translate_vertices(v, x, y, color, dim);
            });
        }

        inline void apply_shaders(ID3D10Device& c, ID3D10VertexShader& vertex_shader, ID3D10PixelShader& pixel_shader)
        {
            c.GSSetShader(nullptr);
            c.VSSetShader(&vertex_shader);
            c.PSSetShader(&pixel_shader);
        }

        inline void apply_shaders(ID3D11DeviceContext& c, ID3D11VertexShader& vertex_shader,
                                  ID3D11PixelShader& pixel_shader)
        {
            c.GSSetShader(nullptr, nullptr, 0);
            c.CSSetShader(nullptr, nullptr, 0);
            c.VSSetShader(&vertex_shader, nullptr, 0);
            c.PSSetShader(&pixel_shader, nullptr, 0);
        }
    }

    template <typename d3dx_traits>
    class d3dx_canvas : public dxgi_canvas
    {
      public:
        using traits = d3dx_traits;

        d3dx_canvas(IDXGISwapChain& swap_chain)
            : swap_chain_(&swap_chain)
        {
            this->device_ = get_device<typename traits::device>(swap_chain);

            auto& d = *this->device_;

            CComPtr<ID3DBlob> vs_blob{};
            this->vertex_shader_ = detail::compile_vertex_shader(d, &vs_blob);
            this->pixel_shader_ = detail::compile_pixel_shader(d);
            this->input_layout_ = detail::create_input_layout(d, *vs_blob);

            this->index_buffer_ = detail::create_index_buffer(d);
            this->vertex_buffer_ = detail::create_vertex_buffer(d);

            this->blend_state_ = detail::create_blend_state(d);
            this->sampler_state_ = detail::create_sampler_state(d);
            this->rasterizer_state_ = detail::create_rasterizer_state(d);
            this->depth_stencil_state_ = detail::create_depth_stencil_state(d);
        }

        d3dx_canvas(IDXGISwapChain& swap_chain, const dimensions dim)
            : d3dx_canvas(swap_chain)
        {
            this->resize(dim);
        }

        void paint(std::span<const uint8_t> image) override
        {
            if (!this->texture_ || image.size() != this->get_buffer_size())
            {
                return;
            }

            const auto context = get_context(*this->device_);

            typename traits::texture2d_desc desc{};
            this->texture_->GetDesc(&desc);

            const auto height = this->get_height();
            const auto row_pitch = image.size() / height;

            if (desc.Usage == traits::USAGE_DYNAMIC &&
                (desc.CPUAccessFlags & traits::CPU_ACCESS_WRITE) == traits::CPU_ACCESS_WRITE)
            {
                detail::access_texture(*this->texture_, [&](void* data, const uint32_t tex_row_pitch) {
                    for (size_t i = 0; i < height; ++i)
                    {
                        auto* src = image.data() + (i * row_pitch);
                        auto* dest = static_cast<uint8_t*>(data) + (i * tex_row_pitch);

                        std::memcpy(dest, src, row_pitch);
                    }
                });
            }
            else if (desc.Usage == traits::USAGE_DEFAULT)
            {
                typename traits::box box{};
                box.top = 0;
                box.left = 0;
                box.front = 0;
                box.back = 1;
                box.right = this->get_width();
                box.bottom = this->get_height();

                get_context(*this->device_)
                    ->UpdateSubresource(this->texture_, 0, &box, image.data(), static_cast<UINT>(row_pitch),
                                        static_cast<UINT>(image.size()));
            }
        }

        void before_resize() override
        {
            this->render_target_view_.Release();
        }

        void draw() const override
        {
            if (!this->texture_)
            {
                return;
            }

            const auto c = get_context(*this->device_);

            detail::context_store _{*c};

            constexpr UINT stride = sizeof(vertex);
            constexpr UINT offset = 0;
            constexpr float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};

            typename traits::viewport view_port = {};
            view_port.Width = static_cast<decltype(view_port.Width)>(this->get_width());
            view_port.Height = static_cast<decltype(view_port.Height)>(this->get_height());
            view_port.MinDepth = 0.0f;
            view_port.MaxDepth = 1.0f;
            view_port.TopLeftX = 0;
            view_port.TopLeftY = 0;

            c->RSSetViewports(1, &view_port);
            c->RSSetState(this->rasterizer_state_);

            c->IASetInputLayout(this->input_layout_);
            c->IASetVertexBuffers(0, 1, &vertex_buffer_.p, &stride, &offset);
            c->IASetIndexBuffer(this->index_buffer_, DXGI_FORMAT_R32_UINT, 0);
            c->IASetPrimitiveTopology(traits::PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            detail::apply_shaders(*c, *this->vertex_shader_, *this->pixel_shader_);

            c->PSSetSamplers(0, 1, &this->sampler_state_.p);
            c->PSSetShaderResources(0, 1, &shader_resource_view_.p);

            c->OMSetBlendState(this->blend_state_, blendFactor, 0xffffffff);
            c->OMSetDepthStencilState(this->depth_stencil_state_, 1);

            c->OMSetRenderTargets(1, &this->render_target_view_.p, nullptr);

            c->DrawIndexed(6, 0, 0);
        }

      private:
        CComPtr<IDXGISwapChain> swap_chain_{};

        CComPtr<typename traits::device> device_{};

        CComPtr<typename traits::render_target_view> render_target_view_{};

        CComPtr<typename traits::vertex_shader> vertex_shader_{};
        CComPtr<typename traits::pixel_shader> pixel_shader_{};
        CComPtr<typename traits::input_layout> input_layout_{};

        CComPtr<typename traits::buffer> index_buffer_{};
        CComPtr<typename traits::buffer> vertex_buffer_{};
        CComPtr<typename traits::blend_state> blend_state_{};
        CComPtr<typename traits::sampler_state> sampler_state_{};
        CComPtr<typename traits::rasterizer_state> rasterizer_state_{};
        CComPtr<typename traits::depth_stencil_state> depth_stencil_state_{};

        CComPtr<typename traits::texture2d> texture_{};
        CComPtr<typename traits::shader_resource_view> shader_resource_view_{};

        void resize_texture(const dimensions new_dimensions) override
        {
            const auto needs_resize =
                new_dimensions != this->get_dimensions() || !this->render_target_view_ || !this->texture_;

            if (new_dimensions.is_zero() || !needs_resize)
            {
                return;
            }

            const auto srgb = detail::is_srgb<traits>(*this->swap_chain_);
            const auto format = srgb ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;

            this->render_target_view_ = detail::create_render_target_view(*this->device_, *this->swap_chain_);

            this->texture_ = detail::create_texture_2d(*this->device_, new_dimensions, format);
            this->shader_resource_view_ = detail::create_shader_resource_view(*this->texture_);

            detail::translate_vertices(*this->vertex_buffer_, 0, 0, ~0UL, new_dimensions);
        }
    };
}
