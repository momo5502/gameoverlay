#include "d3d12_canvas.hpp"
#include <cassert>
#include <stdexcept>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <set>

#include "dxgi_utils.hpp"
#include "utils/concurrency.hpp"
#include "utils/finally.hpp"
#include "utils/hook.hpp"

namespace gameoverlay::dxgi
{
    namespace
    {
        struct vertex
        {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT2 tex_coord;
            COLORREF color;
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
            Texture2D SpriteTex : register(t0);
            SamplerState samLinear : register(s0);

            struct VertexOut {
                float4 Pos : SV_POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            float4 PS(VertexOut pin) : SV_Target {
                return pin.Color * SpriteTex.Sample(samLinear, pin.Tex);
            }
        )";

        CComPtr<ID3DBlob> compile_shader(const std::string& src, const std::string& target,
                                         const std::string& entry_point)
        {
            CComPtr<ID3DBlob> shader_blob{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3DCompile(src.c_str(), src.size(), nullptr, nullptr, nullptr, entry_point.c_str(),
                                  target.c_str(), 0, 0, &shader_blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile shader");
            }

            return shader_blob;
        }

        using queue_map = std::set<CComPtr<ID3D12CommandQueue>>;
        utils::concurrency::container<queue_map> g_command_queue{};

        CComPtr<ID3D12RootSignature> create_root_signature(ID3D12Device& device)
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
            // This is the highest version the sample supports.
            // If CheckFeatureSupport succeeds, the HighestVersion
            // returned will not be greater than this.
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            const auto hr = device.CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
            if (FAILED(hr))
            {
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
            ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

            CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
            rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

            D3D12_STATIC_SAMPLER_DESC sampler = {};
            sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.MipLODBias = 0;
            sampler.MaxAnisotropy = 0;
            sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            sampler.MinLOD = 0.0f;
            sampler.MaxLOD = D3D12_FLOAT32_MAX;
            sampler.ShaderRegister = 0;
            sampler.RegisterSpace = 0;
            sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
                                       D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            CComPtr<ID3DBlob> signature_blob{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                                             &signature_blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to serialize root signature");
            }

            CComPtr<ID3D12RootSignature> root_signature{};
            res = device.CreateRootSignature(0, signature_blob->GetBufferPointer(), signature_blob->GetBufferSize(),
                                             IID_PPV_ARGS(&root_signature));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create root signature");
            }

            return root_signature;
        }

        CComPtr<ID3D12PipelineState> create_pipeline_state(ID3D12Device& device, ID3D12RootSignature& root_signature,
                                                           ID3DBlob& vs_blob, ID3DBlob& ps_blob,
                                                           IDXGISwapChain3& swap_chain)
        {
            D3D12_INPUT_ELEMENT_DESC elements[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            };

            D3D12_INPUT_LAYOUT_DESC layout{};
            layout.NumElements = ARRAYSIZE(elements);
            layout.pInputElementDescs = elements;

            D3D12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            blend_desc.RenderTarget[0].BlendEnable = TRUE;
            blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            DXGI_SWAP_CHAIN_DESC1 desc;
            swap_chain.GetDesc1(&desc);

            CD3DX12_RASTERIZER_DESC rasterizer_desc(D3D12_DEFAULT);
            rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
            rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
            rasterizer_desc.FrontCounterClockwise = TRUE;

            CD3DX12_DEPTH_STENCIL_DESC depth_stencil_desc(D3D12_DEFAULT);
            depth_stencil_desc.DepthEnable = false;
            depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            depth_stencil_desc.StencilEnable = false;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
            pso_desc.InputLayout = layout;
            pso_desc.pRootSignature = &root_signature;
            pso_desc.VS = {vs_blob.GetBufferPointer(), vs_blob.GetBufferSize()};
            pso_desc.PS = {ps_blob.GetBufferPointer(), ps_blob.GetBufferSize()};
            pso_desc.RasterizerState = rasterizer_desc;
            pso_desc.BlendState = blend_desc;
            pso_desc.DepthStencilState = depth_stencil_desc;
            pso_desc.SampleMask = UINT_MAX;
            pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pso_desc.NumRenderTargets = 1;
            pso_desc.RTVFormats[0] = desc.Format;
            pso_desc.SampleDesc.Count = 1;

            CComPtr<ID3D12PipelineState> pipeline_state{};
            auto res = device.CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create pipeline state");
            }

            return pipeline_state;
        }

        CComPtr<ID3D12Resource> create_buffer(ID3D12Device& device, const D3D12_HEAP_PROPERTIES& heap_properties,
                                              const D3D12_RESOURCE_DESC& resource_desc,
                                              D3D12_RESOURCE_STATES initial_state,
                                              const D3D12_CLEAR_VALUE* clear_value = nullptr)
        {
            CComPtr<ID3D12Resource> buffer{};
            auto res = device.CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                                      initial_state, clear_value, IID_PPV_ARGS(&buffer));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create buffer");
            }

            return buffer;
        }

        std::pair<CComPtr<ID3D12Resource>, CComPtr<ID3D12Resource>> create_texture_2d(ID3D12Device& device,
                                                                                      const dimensions dim,
                                                                                      const DXGI_FORMAT format)
        {
            D3D12_HEAP_PROPERTIES heap_properties{};
            heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC resource_desc{};
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resource_desc.Alignment = 0;
            resource_desc.Width = dim.width;
            resource_desc.Height = dim.height;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.MipLevels = 1;
            resource_desc.Format = format;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.SampleDesc.Quality = 0;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            auto t1 = create_buffer(device, heap_properties, resource_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(t1, 0, 1);

            // Create the GPU upload buffer.
            D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            auto t2 = create_buffer(device, uploadHeapProperties, uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

            return {std::move(t1), std::move(t2)};
        }

        CComPtr<ID3D12DescriptorHeap> create_descriptor_heap(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                             D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT num_descriptors)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.NumDescriptors = num_descriptors;
            desc.Type = type;
            desc.Flags = flags;

            CComPtr<ID3D12DescriptorHeap> descriptor_heap{};
            auto res = device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create descriptor heap");
            }

            return descriptor_heap;
        }

        void translate_vertices(ID3D12Resource& vertex_buffer, const int32_t x, const int32_t y, const COLORREF color,
                                const dimensions dim)
        {
            vertex* vertex_data_begin{};
            D3D12_RANGE read_range{0, 0};
            auto res = vertex_buffer.Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to map vertex buffer");
            }

            const auto f_x = static_cast<float>(x);
            const auto f_y = static_cast<float>(y);
            const auto f_width = static_cast<float>(dim.width);
            const auto f_height = static_cast<float>(dim.height);

            const auto w1 = 2.0f * f_x / f_width - 1.0f;
            const auto w2 = 2.0f * (f_x + f_width) / f_width - 1.0f;
            const auto h1 = 1.0f - 2.0f * f_y / f_height;
            const auto h2 = 1.0f - 2.0f * (f_y + f_height) / f_height;

            vertex_data_begin[0] = {DirectX::XMFLOAT3(w1, h1, 0.5f), DirectX::XMFLOAT2(0.0f, 0.0f), color};
            vertex_data_begin[1] = {DirectX::XMFLOAT3(w2, h1, 0.5f), DirectX::XMFLOAT2(1.0f, 0.0f), color};
            vertex_data_begin[2] = {DirectX::XMFLOAT3(w2, h2, 0.5f), DirectX::XMFLOAT2(1.0f, 1.0f), color};
            vertex_data_begin[3] = {DirectX::XMFLOAT3(w1, h2, 0.5f), DirectX::XMFLOAT2(0.0f, 1.0f), color};

            vertex_buffer.Unmap(0, nullptr);
        }
    }

    utils::hook::detour exhook{};

    void STDMETHODCALLTYPE ExecuteCommandListsStub(ID3D12CommandQueue* self, _In_ UINT NumCommandLists,
                                                   _In_reads_(NumCommandLists) ID3D12CommandList* const* ppCommandLists)
    {
        CComPtr<ID3D12Device> dev{};
        self->GetDevice(IID_PPV_ARGS(&dev));

        if (dev)
        {
            g_command_queue.access([&](queue_map& m) {
                m.insert(self); //
            });
        }

        exhook.invoke_stdcall(self, NumCommandLists, ppCommandLists);
    }

    d3d12_canvas::d3d12_canvas(IDXGISwapChain3& swap_chain)
        : swap_chain_(&swap_chain),
          fence_value_(1)
    {
        this->device_ = get_device<ID3D12Device>(swap_chain);

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        static const auto x = [&] {
            CComPtr<ID3D12CommandQueue> command_queue{};
            device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));

            auto* entry = *utils::hook::get_vtable_entry(&*command_queue, &ID3D12CommandQueue::ExecuteCommandLists);
            exhook.create(entry, ExecuteCommandListsStub);
            /*  CreateThread(
                  nullptr, 0,
                  +[](void* ptr) -> DWORD {
                      MessageBoxA(0, 0, 0, 0);
                      exhook.create(ptr, ExecuteCommandListsStub);
                      return 0;
                  },
                  entry, 0, 0);*/
            return 0;
        }();

        device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));
        device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_, nullptr,
                                   IID_PPV_ARGS(&command_list_));

        rtv_heap_ =
            create_descriptor_heap(*device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 2);

        srv_heap_ = create_descriptor_heap(*device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                           D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

        rtv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        srv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain.GetDesc(&swap_chain_desc);

        load_pipeline();
        load_assets();

        device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));

        this->command_list_->Close();
    }

    d3d12_canvas::d3d12_canvas(IDXGISwapChain3& swap_chain, const dimensions dim)
        : d3d12_canvas(swap_chain)
    {
        this->resize(dim);
    }

    void d3d12_canvas::load_pipeline()
    {
        auto vs_blob = compile_shader(vertex_shader_src, "vs_5_0", "VS");
        auto ps_blob = compile_shader(pixel_shader_src, "ps_5_0", "PS");

        root_signature_ = create_root_signature(*device_);
        pipeline_state_ = create_pipeline_state(*device_, *root_signature_, *vs_blob, *ps_blob, *swap_chain_);
    }

    void d3d12_canvas::load_assets()
    {
        const DWORD indices[] = {0, 1, 2, 0, 2, 3};

        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 1;
        heap_properties.VisibleNodeMask = 1;

        /*
        D3D12_RESOURCE_DESC index_buffer_desc{};
        index_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        index_buffer_desc.Alignment = 0;
        index_buffer_desc.Width = sizeof(indices);
        index_buffer_desc.Height = 1;
        index_buffer_desc.DepthOrArraySize = 1;
        index_buffer_desc.MipLevels = 1;
        index_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
        index_buffer_desc.SampleDesc.Count = 1;
        index_buffer_desc.SampleDesc.Quality = 0;
        index_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        index_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        */
        D3D12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));

        index_buffer_ = create_buffer(*device_, heap_properties, indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

        void* index_data_begin{};
        D3D12_RANGE read_range{0, 0};
        index_buffer_->Map(0, &read_range, &index_data_begin);
        std::memcpy(index_data_begin, indices, sizeof(indices));
        index_buffer_->Unmap(0, nullptr);

        D3D12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertex) * 4);

        /*
        D3D12_RESOURCE_DESC vertex_buffer_desc{};
        vertex_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        vertex_buffer_desc.Alignment = 0;
        vertex_buffer_desc.Width = sizeof(vertex) * 4;
        vertex_buffer_desc.Height = 1;
        vertex_buffer_desc.DepthOrArraySize = 1;
        vertex_buffer_desc.MipLevels = 1;
        vertex_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
        vertex_buffer_desc.SampleDesc.Count = 1;
        vertex_buffer_desc.SampleDesc.Quality = 0;
        vertex_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        vertex_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;*/

        vertex_buffer_ = create_buffer(*device_, heap_properties, vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    void d3d12_canvas::resize_texture(const dimensions new_dimensions)
    {
        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = static_cast<FLOAT>(new_dimensions.width);
        viewport_.Height = static_cast<FLOAT>(new_dimensions.height);
        viewport_.MinDepth = D3D12_MIN_DEPTH;
        viewport_.MaxDepth = D3D12_MAX_DEPTH;

        scissor_rect_.left = 0;
        scissor_rect_.top = 0;
        scissor_rect_.right = static_cast<LONG>(new_dimensions.width);
        scissor_rect_.bottom = static_cast<LONG>(new_dimensions.height);

        auto [tex, upload] = create_texture_2d(*device_, new_dimensions, DXGI_FORMAT_R8G8B8A8_UNORM);
        this->texture_ = tex;
        this->upload_buffer_ = upload;

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;

        device_->CreateShaderResourceView(texture_, &srv_desc, srv_heap_->GetCPUDescriptorHandleForHeapStart()); // TODO

        translate_vertices(*vertex_buffer_, 0, 0, ~0UL, new_dimensions);
    }

    void d3d12_canvas::paint(const std::span<const uint8_t> image)
    {
        this->buffer.assign(image.begin(), image.end());
    }

    void d3d12_canvas::paint_i(const std::span<const uint8_t> image) const
    {
        if (!texture_ || image.size() != this->get_buffer_size())
        {
            return;
        }

        D3D12_RESOURCE_BARRIER textureTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            texture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

        command_list_->ResourceBarrier(1, &textureTransitionBarrier);

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = image.data();
        textureData.RowPitch = this->get_width() * 4;
        textureData.SlicePitch = textureData.RowPitch * this->get_height();

        UpdateSubresources(command_list_, texture_, this->upload_buffer_, 0, 0, 1, &textureData);

        D3D12_RESOURCE_BARRIER textureTransitionBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            texture_, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        command_list_->ResourceBarrier(1, &textureTransitionBarrier2);
    }

    void d3d12_canvas::draw() const
    {
        if (!command_queue_)
        {
            g_command_queue.access([&](queue_map& m) {
                if (m.empty())
                {
                    return;
                }

                // command_queue_ = *m.begin();

                for (auto& queue : m)
                {
                    constexpr size_t offset = 0x120;
                    constexpr size_t entry = offset / sizeof(void*);
                    constexpr size_t limit = entry + 20;

                    const auto* values = reinterpret_cast<ID3D12CommandQueue**>(&*this->swap_chain_);

                    for (size_t i = 0; i < limit; ++i)
                    {
                        if (values[i] == queue)
                        {
                            command_queue_ = queue;
                            m.clear();
                            return;
                        }
                    }
                }
            });

            return;
        }

        this->wait_for_gpu();

        command_allocator_->Reset();
        command_list_->Reset(command_allocator_, pipeline_state_);

        populate_command_list();
    }

    void d3d12_canvas::wait_for_gpu() const
    {
        auto completedValue = fence_->GetCompletedValue();

        const UINT64 fenceValue = fence_value_;
        auto hr = command_queue_->Signal(fence_, fenceValue);
        if (FAILED(hr))
        {
            return;
        }

        ++fence_value_;

        completedValue = fence_->GetCompletedValue();

        if (completedValue < fenceValue)
        {
            HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            fence_->SetEventOnCompletion(fenceValue, eventHandle);
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);

            /*hr = fence_->SetEventOnCompletion(fenceValue, fence_event_);
            if (FAILED(hr))
            {
                return;
            }

            WaitForSingleObject(fence_event_, INFINITE);*/
        }
    }

    void d3d12_canvas::after_draw()
    {
        if (!command_queue_)
        {
            return;
        }

        return;
        this->wait_for_gpu();
    }

    void d3d12_canvas::before_resize()
    {
        /* MessageBoxA(0, "before", 0, 0);
         for (auto& tgt : render_targets_)
         {
             tgt.Release();
         }*/
    }

    void d3d12_canvas::populate_command_list() const
    {
        CComPtr<ID3D12Resource> render_target{};

        const auto _ = utils::finally([&] {
            command_list_->Close();

            ID3D12CommandList* ppCommandLists[] = {command_list_};
            command_queue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        });

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap_->GetCPUDescriptorHandleForHeapStart());

        const auto frame_index = swap_chain_->GetCurrentBackBufferIndex();

        // for (UINT i = 0; i < 2; ++i)
        {
            const auto x = swap_chain_->GetBuffer(frame_index, IID_PPV_ARGS(&render_target));
            if (FAILED(x) || !render_target)
            {
                puts("FAIL");
                return;
            }

            D3D12_RESOURCE_DESC desc = render_target->GetDesc();

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = desc.Format; // Match back buffer format!
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;

            device_->CreateRenderTargetView(render_target, &rtvDesc, rtvHandle);

            rtvHandle.Offset(1, rtv_descriptor_size_);
        }

        command_list_->SetGraphicsRootSignature(root_signature_);

        ID3D12DescriptorHeap* ppHeaps[] = {srv_heap_};
        command_list_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        command_list_->SetGraphicsRootDescriptorTable(0, srv_heap_->GetGPUDescriptorHandleForHeapStart());

        command_list_->RSSetViewports(1, &viewport_);
        command_list_->RSSetScissorRects(1, &scissor_rect_);

        if (!this->buffer.empty())
        {
            this->paint_i(this->buffer);
        }

        D3D12_RESOURCE_BARRIER barrier{};
        memset(&barrier, 0, sizeof(barrier));

        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = render_target;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        /* D3D12_RESOURCE_BARRIER rtv1Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
             render_targets_[frame_index_], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);*/
        command_list_->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle2(rtv_heap_->GetCPUDescriptorHandleForHeapStart(), 0,
                                                 rtv_descriptor_size_);
        command_list_->OMSetRenderTargets(1, &rtvHandle2, FALSE, nullptr);

        command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};
        vertex_buffer_view.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
        vertex_buffer_view.SizeInBytes = sizeof(vertex) * 4;
        vertex_buffer_view.StrideInBytes = sizeof(vertex);

        command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view);

        D3D12_INDEX_BUFFER_VIEW index_buffer_view{};
        index_buffer_view.BufferLocation = index_buffer_->GetGPUVirtualAddress();
        index_buffer_view.SizeInBytes = sizeof(DWORD) * 6;
        index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

        command_list_->IASetIndexBuffer(&index_buffer_view);

        command_list_->DrawIndexedInstanced(6, 1, 0, 0, 0);

        // Indicate that the back buffer will now be used to present.
        D3D12_RESOURCE_BARRIER rtv2Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            render_target, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        command_list_->ResourceBarrier(1, &rtv2Barrier);
    }
}
