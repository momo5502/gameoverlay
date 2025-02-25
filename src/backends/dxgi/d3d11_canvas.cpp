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

        CComPtr<ID3DX11Effect> create_shader(ID3D11Device& device)
        {
            constexpr char effect_src[] = "Texture2D SpriteTex;"
                                          "SamplerState samLinear {"
                                          "     Filter = MIN_MAG_MIP_LINEAR;"
                                          "     AddressU = WRAP;"
                                          "     AddressV = WRAP;"
                                          "};"
                                          "struct VertexIn {"
                                          "     float3 PosNdc : POSITION;"
                                          "     float2 Tex    : TEXCOORD;"
                                          "     float4 Color  : COLOR;"
                                          "};"
                                          "struct VertexOut {"
                                          "     float4 PosNdc : SV_POSITION;"
                                          "     float2 Tex    : TEXCOORD;"
                                          "     float4 Color  : COLOR;"
                                          "};"
                                          "VertexOut VS(VertexIn vin) {"
                                          "     VertexOut vout;"
                                          "     vout.PosNdc = float4(vin.PosNdc, 1.0f);"
                                          "     vout.Tex    = vin.Tex;"
                                          "     vout.Color  = vin.Color;"
                                          "     return vout;"
                                          "};"
                                          "float4 PS(VertexOut pin) : SV_Target {"
                                          "     return pin.Color*SpriteTex.Sample(samLinear, pin.Tex);"
                                          "};"
                                          "technique11 SpriteTech {"
                                          "     pass P0 {"
                                          "         SetVertexShader( CompileShader( vs_4_0, VS() ) );"
                                          "         SetHullShader( NULL );"
                                          "         SetDomainShader( NULL );"
                                          "         SetGeometryShader( NULL );"
                                          "         SetPixelShader( CompileShader( ps_4_0, PS() ) );"
                                          "     }"
                                          "}";

            CComPtr<ID3D10Blob> shader_buffer{};

            auto res = D3DCompile(effect_src, sizeof(effect_src), nullptr, nullptr, nullptr, "SpriteTech", "fx_5_0", 0,
                                  0, &shader_buffer, nullptr);

            if (FAILED(res) || !shader_buffer)
            {
                throw std::runtime_error("Failed to compile effect");
            }

            CComPtr<ID3DX11Effect> effect{};
            res = D3DX11CreateEffectFromMemory(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), 0,
                                               &device, &effect);

            if (FAILED(res) || !effect)
            {
                throw std::runtime_error("Failed to create effect");
            }

            return effect;
        }

        CComPtr<ID3D11InputLayout> create_input_layout(ID3D11Device& device, const D3DX11_PASS_DESC& pass_desc)
        {
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            CComPtr<ID3D11InputLayout> input_layout{};
            const auto res = device.CreateInputLayout(layout, ARRAYSIZE(layout), pass_desc.pIAInputSignature,
                                                      pass_desc.IAInputSignatureSize, &input_layout);
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
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
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
            depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
            depth_stencil_desc.StencilEnable = true;
            depth_stencil_desc.StencilReadMask = 0xFF;
            depth_stencil_desc.StencilWriteMask = 0xFF;
            depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
            depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
            depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

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

        void translate_vertices(ID3D11Buffer& vertex_buffer, const int32_t x, const int32_t y, const COLORREF color)
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

            UINT num_viewports = 1;
            D3D11_VIEWPORT viewport{};
            context->RSGetViewports(&num_viewports, &viewport);

            const auto f_x = static_cast<float>(x);
            const auto f_y = static_cast<float>(y);

            const auto f_width = static_cast<float>(viewport.Width);
            const auto f_height = static_cast<float>(viewport.Height);

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
    }

    d3d11_canvas::d3d11_canvas(ID3D11Device& device)
        : device_(&device)
    {
        device.GetImmediateContext(&this->context_);

        this->effect_ = create_shader(device);

        this->effect_technique_ = this->effect_->GetTechniqueByName("SpriteTech");
        this->effect_shader_resource_variable_ = this->effect_->GetVariableByName("SpriteTex")->AsShaderResource();

        D3DX11_PASS_DESC pass_desc{};
        this->effect_technique_->GetPassByIndex(0)->GetDesc(&pass_desc);

        this->input_layout_ = create_input_layout(device, pass_desc);
        this->index_buffer_ = create_index_buffer(device);
        this->vertex_buffer_ = create_vertex_buffer(device);
        this->blend_state_ = create_blend_state(device);
        this->depth_stencil_state_ = create_depth_stencil_state(device);
    }

    d3d11_canvas::d3d11_canvas(ID3D11Device& device, const dimensions dim)
        : d3d11_canvas(device)
    {
        this->resize(dim);
    }

    void d3d11_canvas::resize_texture(const dimensions new_dimensions)
    {
        this->texture_ = create_texture_2d(*this->device_, new_dimensions, DXGI_FORMAT_R8G8B8A8_UNORM);
        this->shader_resource_view_ = create_shader_resource_view(*this->texture_);
    }

    void d3d11_canvas::paint(const std::span<const uint8_t> image)
    {
        if (!this->texture_ || image.size() != this->get_buffer_size())
        {
            return;
        }

        D3D11_TEXTURE2D_DESC desc{};
        this->texture_->GetDesc(&desc);

        const auto row_pitch = image.size() / this->get_height();

        if (desc.Usage == D3D11_USAGE_DYNAMIC &&
            (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) == D3D11_CPU_ACCESS_WRITE)
        {
            D3D11_MAPPED_SUBRESOURCE texmap{};
            if (SUCCEEDED(this->context_->Map(this->texture_, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
            {
                const auto _ = utils::finally([&] {
                    this->context_->Unmap(this->texture_, 0); //
                });

                assert(texmap.RowPitch == row_pitch);
                std::memcpy(texmap.pData, image.data(), image.size());
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

            this->context_->UpdateSubresource(this->texture_, 0, &box, image.data(), static_cast<UINT>(row_pitch),
                                              static_cast<UINT>(image.size()));
        }
    }

    void d3d11_canvas::draw() const
    {
        context_store _{*this->context_};

        auto& c = *this->context_;

        constexpr UINT offset = 0;
        constexpr UINT stride = sizeof(vertex);
        constexpr float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};

        auto* vertex_buffer = &*this->vertex_buffer_;
        translate_vertices(*this->vertex_buffer_, 0, 0, ~0UL);

        this->effect_shader_resource_variable_->SetResource(this->shader_resource_view_);
        this->effect_technique_->GetPassByIndex(0)->Apply(0, &c);

        c.OMSetBlendState(this->blend_state_, blendFactor, 0xffffffff);
        c.OMSetDepthStencilState(this->depth_stencil_state_, 1);
        c.IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        c.IASetIndexBuffer(this->index_buffer_, DXGI_FORMAT_R32_UINT, 0);
        c.IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        c.IASetInputLayout(this->input_layout_);
        c.DrawIndexed(6, 0, 0);
    }
}
