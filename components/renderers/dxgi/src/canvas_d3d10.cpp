#include "std_include.hpp"
#include "canvas_d3d10.hpp"

#include <literally/library.hpp>
using namespace literally::library;

namespace gameoverlay
{
	canvas_d3d10::canvas_d3d10()
	{
		this->reset_resources();
	}

	canvas_d3d10::canvas_d3d10(ID3D10Device* _device)
	{
		this->reset_resources();
		this->initialize(_device);
	}

	canvas_d3d10::~canvas_d3d10()
	{
		this->release_resources();
	}

	ID3D10Device* canvas_d3d10::get_device()
	{
		return this->device;
	}

	void canvas_d3d10::release_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		release_resource(this->effect);
		release_resource(this->texture);
		release_resource(this->shader_resource_view);
		release_resource(this->index_buffer);
		release_resource(this->vertex_buffer);
		release_resource(this->input_layout);
		release_resource(this->depth_stencil_state);
		release_resource(this->blend_state);
		release_resource(this->shader_buffer);
		reset_resource(this->device);

		this->free_buffer();
	}

	void canvas_d3d10::reset_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->bytes_per_pixel = 0;
		this->requires_update = false;

		reset_resource(this->effect);
		reset_resource(this->texture);
		reset_resource(this->shader_resource_view);
		reset_resource(this->index_buffer);
		reset_resource(this->vertex_buffer);
		reset_resource(this->input_layout);
		reset_resource(this->depth_stencil_state);
		reset_resource(this->blend_state);
		reset_resource(this->shader_buffer);
		reset_resource(this->device);
		reset_resource(this->buffer);
	}

	bool canvas_d3d10::create(std::string file)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		release_resource(this->texture);
		release_resource(this->shader_resource_view);

		// Create ShaderResourceView from file
		if (FAILED("d3dx10"_lib.invoke_pascal<HRESULT>("D3DX10CreateShaderResourceViewFromFileA", this->device, file.data(), nullptr, nullptr, &this->shader_resource_view, nullptr)) || !this->shader_resource_view)
		{
			release_resource(this->shader_resource_view);
			return false;
		}

		// Get Texture2D from ShaderResourceView
		this->shader_resource_view->GetResource(reinterpret_cast<ID3D10Resource**>(&this->texture));
		if (!this->texture)
		{
			return false;
		}

		// Set local texture dimension
		D3D10_TEXTURE2D_DESC desc;
		this->texture->GetDesc(&desc);

		this->width = desc.Width;
		this->height = desc.Height;

		return true;
	}

	bool canvas_d3d10::create(uint32_t _width, uint32_t _height, DXGI_FORMAT format, const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		release_resource(this->texture);
		release_resource(this->shader_resource_view);

		// Create Texture2D
		D3D10_TEXTURE2D_DESC desc = { 0 };
		desc.Width = _width;
		desc.Height = _height;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D10_USAGE_DYNAMIC;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		if (FAILED(this->device->CreateTexture2D(&desc, nullptr, &this->texture)) || !this->texture)
		{
			release_resource(this->texture);
			return false;
		}

		D3D10_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		srv_desc.Texture2D.MostDetailedMip = 0;

		if (FAILED(this->device->CreateShaderResourceView(this->texture, &srv_desc, &this->shader_resource_view)) || !this->shader_resource_view)
		{
			release_resource(this->shader_resource_view);
			return false;
		}

		this->width = _width;
		this->height = _height;

		D3D10_MAPPED_TEXTURE2D texmap;
		if (SUCCEEDED(this->texture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &texmap)))
		{
			this->bytes_per_pixel = texmap.RowPitch / this->width;
			this->texture->Unmap(0);
		}

		this->free_buffer();
		this->buffer = new char[this->width * this->height * this->bytes_per_pixel];

		return this->perform_update(_buffer);
	}

	bool canvas_d3d10::resize(uint32_t _width, uint32_t _height)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		if (this->width != _width || this->height != _height)
		{
			if (this->texture)
			{
				D3D10_TEXTURE2D_DESC desc;
				this->texture->GetDesc(&desc);

				if (desc.Usage == D3D10_USAGE_DYNAMIC)
				{
					success = this->create(_width, _height, desc.Format);
				}
			}
		}

		return success;
	}

	bool canvas_d3d10::update(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (_buffer)
		{
			std::memcpy(this->buffer, _buffer, this->width * this->height * this->bytes_per_pixel);
			this->requires_update = true;
		}

		return true;
	}

	bool canvas_d3d10::update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (callback)
		{
			callback(this->buffer, this->width, this->height, this->bytes_per_pixel);
			this->requires_update = true;
		}

		return true;
	}

	bool canvas_d3d10::perform_update(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		if (_buffer && this->texture)
		{
			D3D10_TEXTURE2D_DESC desc;
			this->texture->GetDesc(&desc);

			if (desc.Usage == D3D10_USAGE_DYNAMIC && (desc.CPUAccessFlags & D3D10_CPU_ACCESS_WRITE) == D3D10_CPU_ACCESS_WRITE)
			{
				D3D10_MAPPED_TEXTURE2D texmap;
				if (SUCCEEDED(this->texture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &texmap)))
				{
					for (uint32_t row = 0; row < this->height; ++row)
					{
						std::memcpy(PBYTE(texmap.pData) + row * texmap.RowPitch, PBYTE(_buffer) + (this->bytes_per_pixel * this->width) * row, std::min(texmap.RowPitch, this->bytes_per_pixel * this->width));
					}

					this->texture->Unmap(0);
					success = true;
				}
			}
			else if (desc.Usage == D3D10_USAGE_DEFAULT)
			{
				D3D10_BOX box;
				box.front = 0;
				box.back = 1;
				box.left = 0;
				box.right = this->width;
				box.top = 0;
				box.bottom = this->height;

				this->device->UpdateSubresource(this->texture, 0, &box, _buffer, this->width * this->bytes_per_pixel, this->width * this->height * this->bytes_per_pixel);
				success = true;
			}
		}

		return success;
	}

	bool canvas_d3d10::initialize(ID3D10Device* _device)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->release_resources();

		if (!_device) return false;

		this->device = _device;

		return this->create_resources();
	}

	bool canvas_d3d10::create_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release used resources
		release_resource(this->effect);
		release_resource(this->shader_buffer);
		release_resource(this->vertex_buffer);
		release_resource(this->input_layout);
		release_resource(this->index_buffer);

		// Effect data
		static const char effect_src[] =
			"Texture2D SpriteTex;"
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
			"technique10 SpriteTech {"
			"     pass P0 {"
			"         SetVertexShader( CompileShader( vs_4_0, VS() ) );"
			"         SetGeometryShader( NULL );"
			"         SetPixelShader( CompileShader( ps_4_0, PS() ) );"
			"     }"
			"}";

		// Input layout data
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};

		// Compile Effect
		if (FAILED(D3DCompile(effect_src, sizeof(effect_src), 0, 0, 0, "SpriteTech", "fx_4_0", 0, 0, &this->shader_buffer, 0)) || !this->shader_buffer)
		{
			release_resource(this->shader_buffer);
			return false;
		}

		// Create ShaderBuffer from compiled Effect
		if (FAILED(D3D10CreateEffectFromMemory(this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(), 0, this->device, nullptr, &this->effect)) || !this->effect)
		{
			release_resource(this->effect);
			return false;
		}

		D3D10_PASS_DESC passDesc;
		this->effect_technique = this->effect->GetTechniqueByName("SpriteTech");
		this->effect_shader_resource_variable = this->effect->GetVariableByName("SpriteTex")->AsShaderResource();
		this->effect_technique->GetPassByIndex(0)->GetDesc(&passDesc);

		// Create InputLayout
		if (FAILED(this->device->CreateInputLayout(layout, ARRAYSIZE(layout), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &this->input_layout)) || !this->input_layout)
		{
			release_resource(this->input_layout);
			return false;
		}

		// Create IndexBuffer
		DWORD indices[] =
		{
			0, 1, 2,
			0, 2, 3,
		};

		D3D10_BUFFER_DESC index_buffer_desc;
		ZeroMemory(&index_buffer_desc, sizeof(index_buffer_desc));
		index_buffer_desc.Usage = D3D10_USAGE_DEFAULT;
		index_buffer_desc.ByteWidth = sizeof(DWORD) * 2 * 3;
		index_buffer_desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		index_buffer_desc.CPUAccessFlags = 0;
		index_buffer_desc.MiscFlags = 0;

		D3D10_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = indices;

		if (FAILED(this->device->CreateBuffer(&index_buffer_desc, &iinitData, &this->index_buffer)) || !this->index_buffer)
		{
			release_resource(this->index_buffer);
			return false;
		}

		// Initialize vertex buffer
		D3D10_BUFFER_DESC vertex_buffer_desc;
		ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
		vertex_buffer_desc.Usage = D3D10_USAGE_DYNAMIC;
		vertex_buffer_desc.ByteWidth = sizeof(canvas_d3d10::vertex) * 4;
		vertex_buffer_desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		vertex_buffer_desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		if (FAILED(this->device->CreateBuffer(&vertex_buffer_desc, nullptr, &this->vertex_buffer)) || !this->vertex_buffer)
		{
			release_resource(this->vertex_buffer);
			return false;
		}

		D3D10_BLEND_DESC blend_desc;
		ZeroMemory(&blend_desc, sizeof(D3D10_BLEND_DESC));
		blend_desc.BlendEnable[0] = TRUE;
		blend_desc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
		blend_desc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
		blend_desc.BlendOp = D3D10_BLEND_OP_ADD;
		blend_desc.SrcBlendAlpha = D3D10_BLEND_ZERO;
		blend_desc.DestBlendAlpha = D3D10_BLEND_ZERO;
		blend_desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
		blend_desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(this->device->CreateBlendState(&blend_desc, &this->blend_state)) || !this->blend_state)
		{
			release_resource(this->blend_state);
			return false;
		}

		D3D10_DEPTH_STENCIL_DESC depth_stencil_desc;
		ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
		depth_stencil_desc.DepthEnable = false;
		depth_stencil_desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D10_COMPARISON_LESS;
		depth_stencil_desc.StencilEnable = true;
		depth_stencil_desc.StencilReadMask = 0xFF;
		depth_stencil_desc.StencilWriteMask = 0xFF;
		depth_stencil_desc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
		depth_stencil_desc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
		depth_stencil_desc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
		depth_stencil_desc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

		if (FAILED(this->device->CreateDepthStencilState(&depth_stencil_desc, &this->depth_stencil_state)) || !this->depth_stencil_state)
		{
			release_resource(this->depth_stencil_state);
			return false;
		}

		return true;
	}

	void canvas_d3d10::free_buffer()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (this->buffer)
		{
			delete[] this->buffer;
			this->buffer = nullptr;
		}

		this->requires_update = false;
	}

	bool canvas_d3d10::translate_vertices(int32_t x, int32_t y, uint32_t _width, uint32_t _height, COLORREF color)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		UINT num_viewports = 1;
		D3D10_VIEWPORT viewport;
		void* mapped_data;

		// Calculate VertexBuffer
		if (!this->vertex_buffer || FAILED(this->vertex_buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapped_data))) return false;

		this->device->RSGetViewports(&num_viewports, &viewport);
		canvas_d3d10::vertex* v = reinterpret_cast<canvas_d3d10::vertex*>(mapped_data);

		v[0] = canvas_d3d10::vertex((2.0f * (float)x / viewport.Width - 1.0f), (1.0f - 2.0f * (float)y / viewport.Height), 0.5f, 0.0f, 0.0f, color); // Vertex 1
		v[1] = canvas_d3d10::vertex((2.0f * (float)(x + _width) / viewport.Width - 1.0f), (1.0f - 2.0f * (float)y / viewport.Height), 0.5f, 1.0f, 0.0f, color); // Vertex 2
		v[2] = canvas_d3d10::vertex((2.0f * (float)(x + _width) / viewport.Width - 1.0f), (1.0f - 2.0f * (float)(y + _height) / viewport.Height), 0.5f, 1.0f, 1.0f, color); // Vertex 3
		v[3] = canvas_d3d10::vertex((2.0f * (float)x / viewport.Width - 1.0f), (1.0f - 2.0f * (float)(y + _height) / viewport.Height), 0.5f, 0.0f, 1.0f, color); // Vertex 4

		this->vertex_buffer->Unmap();

		return true;
	}

	void canvas_d3d10::draw(int32_t x, int32_t y, COLORREF color)
	{
		this->draw(x, y, this->width, this->height, color);
	}

	void canvas_d3d10::draw(int32_t x, int32_t y, uint32_t _width, uint32_t _height, COLORREF color)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		if (!this->device) return;

		UINT offset = 0;
		UINT stride = sizeof(canvas_d3d10::vertex);

		if (this->requires_update)
		{
			this->perform_update(this->buffer);
			this->requires_update = false;
		}

		canvas_d3d10::context_store $(this->device);

		this->translate_vertices(x, y, _width, _height, color);

		const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		this->device->OMSetBlendState(this->blend_state, blendFactor, 0xffffffff);
		this->device->OMSetDepthStencilState(this->depth_stencil_state, 1);
		this->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->effect_shader_resource_variable->SetResource(this->shader_resource_view);
		this->effect_technique->GetPassByIndex(0)->Apply(0);
		this->device->IASetIndexBuffer(this->index_buffer, DXGI_FORMAT_R32_UINT, 0);
		this->device->IASetVertexBuffers(0, 1, &this->vertex_buffer, &stride, &offset);
		this->device->IASetInputLayout(this->input_layout);
		this->device->DrawIndexed(6, 0, 0);
	}

	bool canvas_d3d10::is_initialized()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return this->device;
	}

	bool canvas_d3d10::is_loaded()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return (this->texture && this->shader_resource_view);
	}

	ID3D10Texture2D* canvas_d3d10::get_texture()
	{
		return this->texture;
	}
}
