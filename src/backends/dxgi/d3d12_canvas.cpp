#include "d3d12_canvas.hpp"

#include "dxgi_canvas_utils.hpp"

#include <utils/finally.hpp>
#include <utils/concurrency.hpp>

namespace gameoverlay::dxgi
{
    namespace
    {
        CComPtr<ID3DBlob> compile_shader(const std::string_view& src, const std::string& target,
                                         const std::string& entry_point)
        {
            CComPtr<ID3DBlob> shader_blob{};
            CComPtr<ID3DBlob> error_blob{};
            const auto res = D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, entry_point.c_str(),
                                        target.c_str(), 0, 0, &shader_blob, &error_blob);

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to compile shader");
            }

            return shader_blob;
        }

        CComPtr<ID3D12RootSignature> create_root_signature(ID3D12Device& device)
        {
            D3D12_DESCRIPTOR_RANGE range{};
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = 1;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            D3D12_ROOT_PARAMETER root_parameter{};
            root_parameter.DescriptorTable.NumDescriptorRanges = 1;
            root_parameter.DescriptorTable.pDescriptorRanges = &range;
            root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
            sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_desc.MipLODBias = 0;
            sampler_desc.MaxAnisotropy = 0;
            sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            sampler_desc.MinLOD = 0.0f;
            sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
            sampler_desc.ShaderRegister = 0;
            sampler_desc.RegisterSpace = 0;
            sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
            root_signature_desc.NumParameters = 1;
            root_signature_desc.pParameters = &root_parameter;
            root_signature_desc.NumStaticSamplers = 1;
            root_signature_desc.pStaticSamplers = &sampler_desc;
            root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            CComPtr<ID3DBlob> signature_blob{};
            CComPtr<ID3DBlob> error_blob{};
            auto res = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
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
            DXGI_SWAP_CHAIN_DESC1 desc;
            auto res = swap_chain.GetDesc1(&desc);
            if (FAILED(res))
            {
                throw std::runtime_error("Failed to get swap chain desc");
            }

            D3D12_INPUT_ELEMENT_DESC elements[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            };

            D3D12_INPUT_LAYOUT_DESC layout{};
            layout.NumElements = ARRAYSIZE(elements);
            layout.pInputElementDescs = elements;

            D3D12_BLEND_DESC blend_desc{};
            blend_desc.RenderTarget[0].BlendEnable = TRUE;
            blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            D3D12_RASTERIZER_DESC rasterizer_desc{};
            rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
            rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
            rasterizer_desc.FrontCounterClockwise = TRUE;

            D3D12_DEPTH_STENCIL_DESC depth_stencil_desc{};
            depth_stencil_desc.DepthEnable = false;
            depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            depth_stencil_desc.StencilEnable = false;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};
            pipeline_state_desc.InputLayout = layout;
            pipeline_state_desc.pRootSignature = &root_signature;
            pipeline_state_desc.RasterizerState = rasterizer_desc;
            pipeline_state_desc.BlendState = blend_desc;
            pipeline_state_desc.DepthStencilState = depth_stencil_desc;
            pipeline_state_desc.SampleMask = UINT_MAX;
            pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pipeline_state_desc.NumRenderTargets = 1;
            pipeline_state_desc.RTVFormats[0] = desc.Format;
            pipeline_state_desc.SampleDesc.Count = 1;

            pipeline_state_desc.VS = {
                .pShaderBytecode = vs_blob.GetBufferPointer(),
                .BytecodeLength = vs_blob.GetBufferSize(),
            };

            pipeline_state_desc.PS = {
                .pShaderBytecode = ps_blob.GetBufferPointer(),
                .BytecodeLength = ps_blob.GetBufferSize(),
            };

            CComPtr<ID3D12PipelineState> pipeline_state{};
            res = device.CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state));

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
            const auto res = device.CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                                            initial_state, clear_value, IID_PPV_ARGS(&buffer));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to create buffer");
            }

            return buffer;
        }

        D3D12_RESOURCE_DESC get_buffer_descriptor(const uint64_t size)
        {
            D3D12_RESOURCE_DESC buffer_desc{};
            buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            buffer_desc.Width = size;
            buffer_desc.Height = 1;
            buffer_desc.DepthOrArraySize = 1;
            buffer_desc.MipLevels = 1;
            buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
            buffer_desc.SampleDesc.Count = 1;
            buffer_desc.SampleDesc.Quality = 0;
            buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            return buffer_desc;
        }

        uint64_t get_required_intermediate_size(ID3D12Resource& resource, const uint32_t first_subresource = 0,
                                                const uint32_t num_subresources = 1)
        {
            const auto desc = resource.GetDesc();

            CComPtr<ID3D12Device> device{};
            const auto res = resource.GetDevice(IID_PPV_ARGS(&device));

            if (FAILED(res) || !device)
            {
                throw std::runtime_error("Failed to get d3d12 device");
            }

            uint64_t required_size{};
            device->GetCopyableFootprints(&desc, first_subresource, num_subresources, 0, nullptr, nullptr, nullptr,
                                          &required_size);

            return required_size;
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

            const auto upload_buffer_size = get_required_intermediate_size(*t1);
            const auto upload_buffer_desc = get_buffer_descriptor(upload_buffer_size);

            D3D12_HEAP_PROPERTIES upload_heap_properties{};
            upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            upload_heap_properties.CreationNodeMask = 1;
            upload_heap_properties.VisibleNodeMask = 1;

            auto t2 =
                create_buffer(device, upload_heap_properties, upload_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ);

            return {std::move(t1), std::move(t2)};
        }

        CComPtr<ID3D12DescriptorHeap> create_descriptor_heap(ID3D12Device& device,
                                                             const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                             const D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                                                             const uint32_t num_descriptors)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.NumDescriptors = num_descriptors;
            desc.Type = type;
            desc.Flags = flags;

            CComPtr<ID3D12DescriptorHeap> descriptor_heap{};
            const auto res = device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap));

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
            constexpr D3D12_RANGE read_range{0, 0};
            const auto res = vertex_buffer.Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin));

            if (FAILED(res))
            {
                throw std::runtime_error("Failed to map vertex buffer");
            }

            translate_vertices(vertex_data_begin, x, y, color, dim);

            vertex_buffer.Unmap(0, nullptr);
        }

        CComPtr<ID3D12Resource> get_render_target(IDXGISwapChain3& swap_chain)
        {
            CComPtr<ID3D12Resource> render_target{};

            const auto frame_index = swap_chain.GetCurrentBackBufferIndex();
            const auto res = swap_chain.GetBuffer(frame_index, IID_PPV_ARGS(&render_target));

            if (FAILED(res) || !render_target)
            {
                throw std::runtime_error("Failed to get back buffer");
            }

            return render_target;
        }

        void create_render_target_view(ID3D12Device& device, ID3D12Resource& render_target,
                                       const D3D12_CPU_DESCRIPTOR_HANDLE& descriptor_handle)
        {
            const D3D12_RESOURCE_DESC desc = render_target.GetDesc();

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = desc.Format;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;

            device.CreateRenderTargetView(&render_target, &rtvDesc, descriptor_handle);
        }

        void copy_texture(ID3D12Device& device, ID3D12GraphicsCommandList& command_list, ID3D12Resource& destination,
                          ID3D12Resource& source)
        {
            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = &destination;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            command_list.ResourceBarrier(1, &barrier);

            D3D12_TEXTURE_COPY_LOCATION dest_loc{};
            dest_loc.pResource = &destination;
            dest_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dest_loc.PlacedFootprint = {};
            dest_loc.SubresourceIndex = 0;

            const auto desc = destination.GetDesc();
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
            device.GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

            D3D12_TEXTURE_COPY_LOCATION src_loc{};
            src_loc.pResource = &source;
            src_loc.PlacedFootprint = footprint;
            src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

            command_list.CopyTextureRegion(&dest_loc, 0, 0, 0, &src_loc, nullptr);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

            command_list.ResourceBarrier(1, &barrier);
        }

        bool is_srgb(IDXGISwapChain3& swap_chain)
        {
            const auto render_target = get_render_target(swap_chain);
            const auto desc = render_target->GetDesc();
            return is_srgb_format(desc.Format);
        }
    }

    d3d12_canvas::d3d12_canvas(d3d12_command_queue_store& store, IDXGISwapChain3& swap_chain)
        : store_(&store),
          fence_event_(CreateEventA(nullptr, FALSE, FALSE, nullptr)),
          swap_chain_(&swap_chain)
    {
        if (!this->fence_event_)
        {
            throw std::runtime_error("Failed to create fence event");
        }

        this->device_ = get_device<ID3D12Device>(swap_chain);

        this->device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->command_allocator_));
        this->device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->command_allocator_, nullptr,
                                         IID_PPV_ARGS(&this->command_list_));

        this->rtv_heap_ =
            create_descriptor_heap(*this->device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 2);

        this->srv_heap_ = create_descriptor_heap(*this->device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

        this->rtv_descriptor_size_ = this->device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        this->srv_descriptor_size_ =
            this->device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain.GetDesc(&swap_chain_desc);

        this->load_pipeline();
        this->load_assets();

        this->device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence_));

        this->command_list_->Close();
    }

    d3d12_canvas::d3d12_canvas(d3d12_command_queue_store& store, IDXGISwapChain3& swap_chain, const dimensions dim)
        : d3d12_canvas(store, swap_chain)
    {
        this->resize(dim);
    }

    void d3d12_canvas::load_pipeline()
    {
        const auto vs_blob = compile_shader(vertex_shader_src, "vs_5_0", "VS");
        const auto ps_blob = compile_shader(pixel_shader_src, "ps_5_0", "PS");

        this->root_signature_ = create_root_signature(*this->device_);
        this->pipeline_state_ =
            create_pipeline_state(*this->device_, *this->root_signature_, *vs_blob, *ps_blob, *this->swap_chain_);
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

        const auto index_buffer_desc = get_buffer_descriptor(sizeof(indices));

        this->index_buffer_ =
            create_buffer(*this->device_, heap_properties, index_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ);

        void* index_data_begin{};
        constexpr D3D12_RANGE read_range{0, 0};
        this->index_buffer_->Map(0, &read_range, &index_data_begin);
        std::memcpy(index_data_begin, indices, sizeof(indices));
        this->index_buffer_->Unmap(0, nullptr);

        const auto vertex_buffer_desc = get_buffer_descriptor(sizeof(vertex) * 4);
        this->vertex_buffer_ =
            create_buffer(*this->device_, heap_properties, vertex_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    void d3d12_canvas::resize_texture(const dimensions new_dimensions)
    {
        this->viewport_.TopLeftX = 0;
        this->viewport_.TopLeftY = 0;
        this->viewport_.Width = static_cast<FLOAT>(new_dimensions.width);
        this->viewport_.Height = static_cast<FLOAT>(new_dimensions.height);
        this->viewport_.MinDepth = D3D12_MIN_DEPTH;
        this->viewport_.MaxDepth = D3D12_MAX_DEPTH;

        this->scissor_rect_.left = 0;
        this->scissor_rect_.top = 0;
        this->scissor_rect_.right = static_cast<LONG>(new_dimensions.width);
        this->scissor_rect_.bottom = static_cast<LONG>(new_dimensions.height);

        const auto format = is_srgb(*this->swap_chain_) //
                                ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
                                : DXGI_FORMAT_B8G8R8A8_UNORM;

        auto [tex, upload] = create_texture_2d(*this->device_, new_dimensions, format);
        this->texture_ = tex;
        this->upload_buffer_ = upload;

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Format = format;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;

        const auto handle = this->srv_heap_->GetCPUDescriptorHandleForHeapStart();
        this->device_->CreateShaderResourceView(this->texture_, &srv_desc, handle);

        translate_vertices(*this->vertex_buffer_, 0, 0, ~0UL, new_dimensions);
    }

    void d3d12_canvas::paint(const std::span<const uint8_t> image)
    {
        if (!this->upload_buffer_ || !this->texture_ || image.size() != this->get_buffer_size())
        {
            return;
        }

        const auto desc = this->texture_->GetDesc();
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        this->device_->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

        void* destination{};
        constexpr D3D12_RANGE read_range{0, 0};

        const auto res = this->upload_buffer_->Map(0, &read_range, &destination);
        if (FAILED(res))
        {
            throw std::runtime_error("Failed to update image");
        }

        const auto height = this->get_height();
        const auto row_pitch = this->get_width() * 4;

        for (size_t i = 0; i < height; ++i)
        {
            auto* src = image.data() + (i * row_pitch);
            auto* dest = static_cast<uint8_t*>(destination) + footprint.Offset + (i * footprint.Footprint.RowPitch);

            std::memcpy(dest, src, row_pitch);
        }

        this->upload_buffer_->Unmap(0, nullptr);
    }

    void d3d12_canvas::draw() const
    {
        if (!this->texture_)
        {
            return;
        }

        if (!this->command_queue_)
        {
            this->command_queue_ = this->store_->find(*this->swap_chain_);

            if (!this->command_queue_)
            {
                return;
            }
        }

        const auto completed_value = this->fence_->GetCompletedValue();

        if (completed_value < this->fence_value_)
        {
            ResetEvent(this->fence_event_);
            this->fence_->SetEventOnCompletion(this->fence_value_, this->fence_event_);
            WaitForSingleObject(this->fence_event_, INFINITE);
        }

        this->command_allocator_->Reset();
        this->command_list_->Reset(this->command_allocator_, this->pipeline_state_);

        {
            const auto _ = utils::finally([&] {
                this->command_list_->Close(); //
            });

            this->populate_command_list();
        }

        ID3D12CommandList* command_list = this->command_list_;
        this->command_queue_->ExecuteCommandLists(1, &command_list);

        const UINT64 target_fence_value = ++this->fence_value_;
        const auto res = this->command_queue_->Signal(fence_, target_fence_value);
        if (FAILED(res))
        {
            throw std::runtime_error("Failed to signal fence");
        }
    }

    void d3d12_canvas::populate_command_list() const
    {
        const auto render_target = get_render_target(*this->swap_chain_);
        const auto render_target_view_handle = this->rtv_heap_->GetCPUDescriptorHandleForHeapStart();

        create_render_target_view(*this->device_, *render_target, render_target_view_handle);

        copy_texture(*this->device_, *this->command_list_, *this->texture_, *this->upload_buffer_);

        this->command_list_->SetGraphicsRootSignature(this->root_signature_);
        this->command_list_->SetDescriptorHeaps(1, &this->srv_heap_.p);
        this->command_list_->SetGraphicsRootDescriptorTable(0, this->srv_heap_->GetGPUDescriptorHandleForHeapStart());
        this->command_list_->RSSetViewports(1, &this->viewport_);
        this->command_list_->RSSetScissorRects(1, &this->scissor_rect_);

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = render_target;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        this->command_list_->ResourceBarrier(1, &barrier);

        this->command_list_->OMSetRenderTargets(1, &render_target_view_handle, FALSE, nullptr);
        this->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};
        vertex_buffer_view.BufferLocation = this->vertex_buffer_->GetGPUVirtualAddress();
        vertex_buffer_view.SizeInBytes = sizeof(vertex) * 4;
        vertex_buffer_view.StrideInBytes = sizeof(vertex);

        D3D12_INDEX_BUFFER_VIEW index_buffer_view{};
        index_buffer_view.BufferLocation = this->index_buffer_->GetGPUVirtualAddress();
        index_buffer_view.SizeInBytes = sizeof(DWORD) * 6;
        index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

        this->command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        this->command_list_->IASetIndexBuffer(&index_buffer_view);

        this->command_list_->DrawIndexedInstanced(6, 1, 0, 0, 0);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        this->command_list_->ResourceBarrier(1, &barrier);
    }
}
