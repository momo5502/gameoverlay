#include "d3d11_canvas.hpp"

#include <cassert>
#include <stdexcept>

#include <utils/finally.hpp>

namespace gameoverlay::dxgi
{
    namespace
    {
        struct vertex
        {
            vertex() = default;
            vertex(const float x, const float y, const float z, const float u, const float v, const COLORREF col)
                : pos(x, y, z),
                  tex_coord(u, v),
                  color(col)
            {
            }

            DirectX::XMFLOAT3 pos{};
            DirectX::XMFLOAT2 tex_coord{};
            COLORREF color{};
        };

        struct context_store
        {
            UINT ref{};
            CComPtr<ID3D11DeviceContext> context{};
            CComPtr<ID3D11DepthStencilState> depth_stencil_state{};

            context_store(ID3D11DeviceContext& c)
                : context(&c)
            {
                this->context->OMGetDepthStencilState(&this->depth_stencil_state, &this->ref);
            }

            ~context_store()
            {
                this->context->OMSetDepthStencilState(this->depth_stencil_state, this->ref);
            }
        };

        constexpr char vertex_shader_src[] = R"(
            struct VertexIn {
                float3 Pos : POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            struct VertexOut {
                float4 Pos : SV_POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            VertexOut VS(VertexIn vin) {
                VertexOut vout;
                vout.Pos = float4(vin.Pos, 1.0f);
                vout.Tex = vin.Tex;
                vout.Color = vin.Color;
                return vout;
            }
        )";

        constexpr char pixel_shader_src[] = R"(
            Texture2D SpriteTex;
            SamplerState samLinear;

            struct VertexOut {
                float4 Pos : SV_POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            float4 PS(VertexOut pin) : SV_Target {
                return pin.Color * SpriteTex.Sample(samLinear, pin.Tex);
            }
        )";

        CComPtr<ID3D11VertexShader> compile_vertex_shader(ID3D11Device& device, ID3DBlob** blob)
        {
            CComPtr<ID3D11VertexShader> shader{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3DCompile(vertex_shader_src, sizeof(vertex_shader_src), nullptr, nullptr, nullptr, "VS",
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

        CComPtr<ID3D11PixelShader> compile_pixel_shader(ID3D11Device& device)
        {
            CComPtr<ID3D11PixelShader> shader{};
            CComPtr<ID3DBlob> shader_blob{};
            CComPtr<ID3DBlob> error_blob{};

            auto res = D3DCompile(pixel_shader_src, sizeof(pixel_shader_src), nullptr, nullptr, nullptr, "PS", "ps_5_0",
                                  0, 0, &shader_blob, &error_blob);

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

        CComPtr<ID3D11InputLayout> create_input_layout(ID3D11Device& device, ID3DBlob& vs_blob)
        {
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            CComPtr<ID3D11InputLayout> input_layout{};
            const auto res = device.CreateInputLayout(layout, ARRAYSIZE(layout), vs_blob.GetBufferPointer(),
                                                      vs_blob.GetBufferSize(), &input_layout);
            if (FAILED(res) || !input_layout)
            {
                throw std::runtime_error("Failed to create input layout");
            }

            return input_layout;
        }

        CComPtr<ID3D11Buffer> create_buffer(ID3D11Device& device, const D3D11_BUFFER_DESC& buffer_desc,
                                            const D3D11_SUBRESOURCE_DATA* init_data = nullptr)
        {
            CComPtr<ID3D11Buffer> buffer{};
            const auto res = device.CreateBuffer(&buffer_desc, init_data, &buffer);

            if (FAILED(res) || !buffer)
            {
                return {};
            }

            return buffer;
        }

        CComPtr<ID3D11Buffer> create_index_buffer(ID3D11Device& device)
        {
            const DWORD indices[] = {
                0, 1, 2, 0, 2, 3,
            };

            D3D11_BUFFER_DESC index_buffer_desc{};
            index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            index_buffer_desc.ByteWidth = sizeof(DWORD) * 2 * 3;
            index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            index_buffer_desc.CPUAccessFlags = 0;
            index_buffer_desc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA iinit_data;
            iinit_data.pSysMem = indices;

            auto index_buffer = create_buffer(device, index_buffer_desc, &iinit_data);

            if (!index_buffer)
            {
                throw std::runtime_error("Failed to create index buffer");
            }

            return index_buffer;
        }

        CComPtr<ID3D11Buffer> create_vertex_buffer(ID3D11Device& device)
        {
            D3D11_BUFFER_DESC vertex_buffer_desc{};
            vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
            vertex_buffer_desc.ByteWidth = sizeof(vertex) * 4;
            vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            auto index_buffer = create_buffer(device, vertex_buffer_desc);
            if (!index_buffer)
            {
                throw std::runtime_error("Failed to create index buffer");
            }

            return index_buffer;
        }

        CComPtr<ID3D11BlendState> create_blend_state(ID3D11Device& device)
        {
            D3D11_BLEND_DESC blend_desc{};
            blend_desc.RenderTarget[0].BlendEnable = TRUE;
            blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            CComPtr<ID3D11BlendState> blend_state{};
            const auto res = device.CreateBlendState(&blend_desc, &blend_state);

            if (FAILED(res) || !blend_state)
            {
                throw std::runtime_error("Failed to create blend state");
            }

            return blend_state;
        }

        CComPtr<ID3D11DepthStencilState> create_depth_stencil_state(ID3D11Device& device)
        {
            D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
            depth_stencil_desc.DepthEnable = false;
            depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
            depth_stencil_desc.StencilEnable = false;

            CComPtr<ID3D11DepthStencilState> depth_stencil_state{};
            const auto res = device.CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);

            if (FAILED(res) || !depth_stencil_state)
            {
                throw std::runtime_error("Failed to create depth stencil state");
            }

            return depth_stencil_state;
        }

        CComPtr<ID3D11Texture2D> create_texture_2d(ID3D11Device& device, const dimensions dim, const DXGI_FORMAT format)
        {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = dim.width;
            desc.Height = dim.height;
            desc.MipLevels = desc.ArraySize = 1;
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            CComPtr<ID3D11Texture2D> texture{};
            const auto res = device.CreateTexture2D(&desc, nullptr, &texture);

            if (FAILED(res) || !texture)
            {
                throw std::runtime_error("Failed to create texture");
            }

            return texture;
        }

        CComPtr<ID3D11ShaderResourceView> create_shader_resource_view(ID3D11Texture2D& texture)
        {
            CComPtr<ID3D11Device> device{};
            texture.GetDevice(&device);

            D3D11_TEXTURE2D_DESC desc{};
            texture.GetDesc(&desc);

            D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = desc.Format;
            srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MipLevels = 1;
            srv_desc.Texture2D.MostDetailedMip = 0;

            CComPtr<ID3D11ShaderResourceView> shader_resource_view{};
            const auto res = device->CreateShaderResourceView(&texture, &srv_desc, &shader_resource_view);

            if (FAILED(res) || !shader_resource_view)
            {
                throw std::runtime_error("Failed to create shader resource view");
            }

            return shader_resource_view;
        }

        CComPtr<ID3D11RasterizerState> create_rasterizer_state(ID3D11Device& device)
        {
            D3D11_RASTERIZER_DESC rasterizer_desc{};
            rasterizer_desc.FillMode = D3D11_FILL_SOLID;
            rasterizer_desc.CullMode = D3D11_CULL_NONE;
            rasterizer_desc.FrontCounterClockwise = TRUE;

            CComPtr<ID3D11RasterizerState> rasterizer_state{};
            const auto res = device.CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

            if (FAILED(res) || !rasterizer_state)
            {
                throw std::runtime_error("Failed to create rasterizer state");
            }

            return rasterizer_state;
        }

        CComPtr<ID3D11RenderTargetView> create_render_target_view(ID3D11Device& device, IDXGISwapChain& swap_chain)
        {
            CComPtr<ID3D11Texture2D> back_buffer{};
            auto res = swap_chain.GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));

            if (FAILED(res) || !back_buffer)
            {
                throw std::runtime_error("Failed to get back buffer");
            }

            CComPtr<ID3D11RenderTargetView> render_target_view{};
            res = device.CreateRenderTargetView(back_buffer, nullptr, &render_target_view);

            if (FAILED(res) || !back_buffer)
            {
                throw std::runtime_error("Failed to get back buffer");
            }

            return render_target_view;
        }

        CComPtr<ID3D11SamplerState> create_sampler_state(ID3D11Device& device)
        {
            D3D11_SAMPLER_DESC sampler_desc{};
            sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            sampler_desc.MinLOD = 0;
            sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

            CComPtr<ID3D11SamplerState> sampler_state{};
            const auto res = device.CreateSamplerState(&sampler_desc, &sampler_state);

            if (FAILED(res) || !sampler_state)
            {
                throw std::runtime_error("Failed to create sampler state");
            }

            return sampler_state;
        }

        void translate_vertices(ID3D11Buffer& vertex_buffer, const int32_t x, const int32_t y, const COLORREF color,
                                const dimensions dim)
        {
            CComPtr<ID3D11Device> device{};
            vertex_buffer.GetDevice(&device);

            CComPtr<ID3D11DeviceContext> context{};
            device->GetImmediateContext(&context);

            D3D11_MAPPED_SUBRESOURCE mapped_data{};
            if (FAILED(context->Map(&vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                context->Unmap(&vertex_buffer, 0); //
            });

            const auto f_x = static_cast<float>(x);
            const auto f_y = static_cast<float>(y);

            const auto f_width = static_cast<float>(dim.width);
            const auto f_height = static_cast<float>(dim.height);

            const auto w1 = 2.0f * f_x / f_width - 1.0f;
            const auto w2 = 2.0f * (f_x + f_width) / f_width - 1.0f;

            const auto h1 = 1.0f - 2.0f * f_y / f_height;
            const auto h2 = 1.0f - 2.0f * (f_y + f_height) / f_height;

            auto* v = static_cast<vertex*>(mapped_data.pData);

            v[0] = vertex(w1, h1, 0.5f, 0.0f, 0.0f, color);
            v[1] = vertex(w2, h1, 0.5f, 1.0f, 0.0f, color);
            v[2] = vertex(w2, h2, 0.5f, 1.0f, 1.0f, color);
            v[3] = vertex(w1, h2, 0.5f, 0.0f, 1.0f, color);
        }

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
    }

    d3d11_canvas::d3d11_canvas(IDXGISwapChain& swap_chain)
        : swap_chain_(&swap_chain)
    {
        this->device_ = get_device<ID3D11Device>(swap_chain);

        auto& d = *this->device_;

        CComPtr<ID3DBlob> vs_blob{};
        this->vertex_shader_ = compile_vertex_shader(d, &vs_blob);
        this->pixel_shader_ = compile_pixel_shader(d);
        this->input_layout_ = create_input_layout(d, *vs_blob);

        this->index_buffer_ = create_index_buffer(d);
        this->vertex_buffer_ = create_vertex_buffer(d);

        this->blend_state_ = create_blend_state(d);
        this->sampler_state_ = create_sampler_state(d);
        this->rasterizer_state_ = create_rasterizer_state(d);
        this->depth_stencil_state_ = create_depth_stencil_state(d);

        this->render_target_view_ = create_render_target_view(d, *this->swap_chain_);
    }

    d3d11_canvas::d3d11_canvas(IDXGISwapChain& swap_chain, const dimensions dim)
        : d3d11_canvas(swap_chain)
    {
        this->resize(dim);
    }

    CComPtr<ID3D11DeviceContext> d3d11_canvas::get_context() const
    {
        CComPtr<ID3D11DeviceContext> context{};
        this->device_->GetImmediateContext(&context);

        return context;
    }

    void d3d11_canvas::resize_texture(const dimensions new_dimensions)
    {
        if (new_dimensions.is_zero())
        {
            return;
        }

        this->texture_ = create_texture_2d(*this->device_, new_dimensions, DXGI_FORMAT_R8G8B8A8_UNORM);
        this->shader_resource_view_ = create_shader_resource_view(*this->texture_);

        translate_vertices(*this->vertex_buffer_, 0, 0, ~0UL, new_dimensions);
    }

    void d3d11_canvas::paint(const std::span<const uint8_t> image)
    {
        if (!this->texture_ || image.size() != this->get_buffer_size())
        {
            return;
        }

        const auto context = this->get_context();

        D3D11_TEXTURE2D_DESC desc{};
        this->texture_->GetDesc(&desc);

        const auto height = this->get_height();
        const auto row_pitch = image.size() / height;

        if (desc.Usage == D3D11_USAGE_DYNAMIC &&
            (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) == D3D11_CPU_ACCESS_WRITE)
        {
            D3D11_MAPPED_SUBRESOURCE texmap{};
            if (SUCCEEDED(context->Map(this->texture_, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
            {
                const auto _ = utils::finally([&] {
                    context->Unmap(this->texture_, 0); //
                });

                for (size_t i = 0; i < height; ++i)
                {
                    auto* src = image.data() + (i * row_pitch);
                    auto* dest = static_cast<uint8_t*>(texmap.pData) + (i * texmap.RowPitch);

                    std::memcpy(dest, src, row_pitch);
                }
            }
        }
        else if (desc.Usage == D3D11_USAGE_DEFAULT)
        {
            D3D11_BOX box{};
            box.top = 0;
            box.left = 0;
            box.front = 0;
            box.back = 1;
            box.right = this->get_width();
            box.bottom = this->get_height();

            context->UpdateSubresource(this->texture_, 0, &box, image.data(), static_cast<UINT>(row_pitch),
                                       static_cast<UINT>(image.size()));
        }
    }

    void d3d11_canvas::draw() const
    {
        const auto c = this->get_context();

        context_store _{*c};

        constexpr UINT stride = sizeof(vertex);
        constexpr UINT offset = 0;
        constexpr float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};

        D3D11_VIEWPORT view_port = {};
        view_port.Width = static_cast<FLOAT>(this->get_width());
        view_port.Height = static_cast<FLOAT>(this->get_height());
        view_port.MinDepth = 0.0f;
        view_port.MaxDepth = 1.0f;
        view_port.TopLeftX = 0;
        view_port.TopLeftY = 0;

        c->RSSetViewports(1, &view_port);
        c->RSSetState(this->rasterizer_state_);

        c->GSSetShader(nullptr, nullptr, 0);
        c->CSSetShader(nullptr, nullptr, 0);

        c->IASetInputLayout(this->input_layout_);
        c->IASetVertexBuffers(0, 1, &vertex_buffer_.p, &stride, &offset);
        c->IASetIndexBuffer(this->index_buffer_, DXGI_FORMAT_R32_UINT, 0);
        c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        c->VSSetShader(this->vertex_shader_, nullptr, 0);

        c->PSSetSamplers(0, 1, &this->sampler_state_.p);
        c->PSSetShader(this->pixel_shader_, nullptr, 0);
        c->PSSetShaderResources(0, 1, &shader_resource_view_.p);

        c->OMSetBlendState(this->blend_state_, blendFactor, 0xffffffff);
        c->OMSetDepthStencilState(this->depth_stencil_state_, 1);
        c->OMSetRenderTargets(1, &this->render_target_view_.p, nullptr);

        c->DrawIndexed(6, 0, 0);
    }
}
