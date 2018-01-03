#include "std_include.hpp"
#include "canvas_d3d11.hpp"

namespace gameoverlay
{
	canvas_d3d11::canvas_d3d11()
	{
		this->resetResources();
	}

	canvas_d3d11::canvas_d3d11(ID3D11Device* pDevice)
	{
		this->resetResources();
		this->initialize(pDevice);
	}

	canvas_d3d11::~canvas_d3d11()
	{
		this->releaseResources();
	}

	void canvas_d3d11::releaseResources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		ReleaseResource(this->effect);
		ReleaseResource(this->texture);
		ReleaseResource(this->shaderResourceView);
		ReleaseResource(this->indexBuffer);
		ReleaseResource(this->vertexBuffer);
		ReleaseResource(this->inputLayout);
		ReleaseResource(this->depthStencilState);
		ReleaseResource(this->blendState);
		ReleaseResource(this->shaderBuffer);
		ResetResource(this->device);
		ResetResource(this->context);

		this->freeBuffer();
	}

	void canvas_d3d11::resetResources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->bytesPerPixel = 0;
		this->requiresUpdate = false;

		ResetResource(this->effect);
		ResetResource(this->texture);
		ResetResource(this->shaderResourceView);
		ResetResource(this->indexBuffer);
		ResetResource(this->vertexBuffer);
		ResetResource(this->inputLayout);
		ResetResource(this->depthStencilState);
		ResetResource(this->blendState);
		ResetResource(this->shaderBuffer);
		ResetResource(this->device);
		ResetResource(this->context);
		ResetResource(this->buffer);
	}

	bool canvas_d3d11::create(std::string file)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		ReleaseResource(this->texture);
		ReleaseResource(this->shaderResourceView);

		// Create ShaderResourceView from file
		std::wstring wideFile(file.begin(), file.end());
		if (FAILED(DirectX::CreateWICTextureFromFile(this->device, wideFile.data(), NULL, &this->shaderResourceView)) || !this->shaderResourceView)
		{
			ReleaseResource(this->shaderResourceView);
			return false;
		}

		// Get Texture2D from ShaderResourceView
		this->shaderResourceView->GetResource(reinterpret_cast<ID3D11Resource**>(&this->texture));
		if (!this->texture)
		{
			return false;
		}

		// Set local texture dimension
		D3D11_TEXTURE2D_DESC desc;
		this->texture->GetDesc(&desc);

		this->width = desc.Width;
		this->height = desc.Height;

		return true;
	}

	bool canvas_d3d11::create(uint32_t _width, uint32_t _height, DXGI_FORMAT format, const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		ReleaseResource(this->texture);
		ReleaseResource(this->shaderResourceView);

		// Create Texture2D
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = _width;
		desc.Height = _height;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		if (FAILED(this->device->CreateTexture2D(&desc, NULL, &this->texture)) || !this->texture)
		{
			ReleaseResource(this->texture);
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		if (FAILED(this->device->CreateShaderResourceView(this->texture, &srvDesc, &this->shaderResourceView)) || !this->shaderResourceView)
		{
			ReleaseResource(this->shaderResourceView);
			return false;
		}

		this->width = _width;
		this->height = _height;

		D3D11_MAPPED_SUBRESOURCE texmap;
		if (SUCCEEDED(this->context->Map(this->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
		{
			this->bytesPerPixel = texmap.RowPitch / this->width;
			this->context->Unmap(this->texture, 0);
		}

		this->freeBuffer();
		this->buffer = this->allocator.allocate(this->width * this->height * this->bytesPerPixel);

		return this->performUpdate(_buffer);
	}

	bool canvas_d3d11::resize(uint32_t _width, uint32_t _height)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		if (this->width != _width || this->height != _height)
		{
			if (this->texture)
			{
				D3D11_TEXTURE2D_DESC desc;
				this->texture->GetDesc(&desc);

				if (desc.Usage == D3D11_USAGE_DYNAMIC)
				{
					success = this->create(_width, _height, desc.Format);
				}
			}
		}

		return success;
	}

	bool canvas_d3d11::update(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (_buffer)
		{
			std::memcpy(this->buffer, _buffer, this->width * this->height * this->bytesPerPixel);
			this->requiresUpdate = true;
		}

		return true;
	}

	bool canvas_d3d11::update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (callback)
		{
			callback(this->buffer, this->width, this->height, this->bytesPerPixel);
			this->requiresUpdate = true;
		}

		return true;
	}

	bool canvas_d3d11::performUpdate(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		if (_buffer && this->texture)
		{
			D3D11_TEXTURE2D_DESC desc;
			this->texture->GetDesc(&desc);

			if (desc.Usage == D3D11_USAGE_DYNAMIC && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) == D3D11_CPU_ACCESS_WRITE)
			{
				D3D11_MAPPED_SUBRESOURCE texmap;
				if (SUCCEEDED(this->context->Map(this->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
				{
					for (uint32_t row = 0; row < this->height; ++row)
					{
						std::memcpy(PBYTE(texmap.pData) + row * texmap.RowPitch, PBYTE(_buffer) + (this->bytesPerPixel * this->width) * row, std::min(texmap.RowPitch, this->bytesPerPixel * this->width));
					}

					this->context->Unmap(this->texture, 0);
					success = true;
				}
			}
			else if (desc.Usage == D3D11_USAGE_DEFAULT)
			{
				D3D11_BOX box;
				box.front = 0;
				box.back = 1;
				box.left = 0;
				box.right = this->width;
				box.top = 0;
				box.bottom = this->height;

				this->context->UpdateSubresource(this->texture, 0, &box, _buffer, this->width * this->bytesPerPixel, this->width * this->height * this->bytesPerPixel);
				success = true;
			}
		}

		return success;
	}

	bool canvas_d3d11::initialize(ID3D11Device* pDevice)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->releaseResources();

		if (!pDevice) return false;

		this->device = pDevice;
		this->device->GetImmediateContext(&this->context);

		return this->createResources();
	}

	bool canvas_d3d11::createResources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release used resources
		ReleaseResource(this->effect);
		ReleaseResource(this->shaderBuffer);
		ReleaseResource(this->vertexBuffer);
		ReleaseResource(this->inputLayout);
		ReleaseResource(this->indexBuffer);

		// Effect data
		const char effectSrc[] =
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
			"technique11 SpriteTech {"
			"     pass P0 {"
			"         SetVertexShader( CompileShader( vs_4_0, VS() ) );"
			"         SetHullShader( NULL );"
			"         SetDomainShader( NULL );"
			"         SetGeometryShader( NULL );"
			"         SetPixelShader( CompileShader( ps_4_0, PS() ) );"
			"     }"
			"}";

		// Input layout data
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		// Compile Effect
		if (FAILED(D3DCompile(effectSrc, sizeof(effectSrc), 0, 0, 0, "SpriteTech", "fx_5_0", 0, 0, &this->shaderBuffer, 0)) || !this->shaderBuffer)
		{
			ReleaseResource(this->shaderBuffer);
			return false;
		}

		// Create ShaderBuffer from compiled Effect
		if (FAILED(D3DX11CreateEffectFromMemory(this->shaderBuffer->GetBufferPointer(), this->shaderBuffer->GetBufferSize(), 0, this->device, &this->effect)) || !this->effect)
		{
			ReleaseResource(this->effect);
			return false;
		}

		D3DX11_PASS_DESC passDesc;
		this->effectTechnique = this->effect->GetTechniqueByName("SpriteTech");
		this->effectShaderResourceVariable = this->effect->GetVariableByName("SpriteTex")->AsShaderResource();
		this->effectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		// Create InputLayout
		if (FAILED(this->device->CreateInputLayout(layout, ARRAYSIZE(layout), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &this->inputLayout)) || !this->inputLayout)
		{
			ReleaseResource(this->inputLayout);
			return false;
		}

		// Create IndexBuffer
		DWORD indices[] =
		{
			0, 1, 2,
			0, 2, 3,
		};

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = indices;

		if (FAILED(this->device->CreateBuffer(&indexBufferDesc, &iinitData, &this->indexBuffer)) || !this->indexBuffer)
		{
			ReleaseResource(this->indexBuffer);
			return false;
		}

		// Initialize vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(canvas_d3d11::Vertex) * 4;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(this->device->CreateBuffer(&vertexBufferDesc, NULL, &this->vertexBuffer)) || !this->vertexBuffer)
		{
			ReleaseResource(this->vertexBuffer);
			return false;
		}

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(this->device->CreateBlendState(&blendDesc, &this->blendState)) || !this->blendState)
		{
			ReleaseResource(this->blendState);
			return false;
		}

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable = false;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		if (FAILED(this->device->CreateDepthStencilState(&depthStencilDesc, &this->depthStencilState)) || !this->depthStencilState)
		{
			ReleaseResource(this->depthStencilState);
			return false;
		}

		return true;
	}

	void canvas_d3d11::freeBuffer()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (this->buffer)
		{
			this->allocator.free(buffer);
			this->buffer = nullptr;
		}

		this->requiresUpdate = false;
	}

	bool canvas_d3d11::translateVertices(int32_t x, int32_t y, uint32_t _width, uint32_t _height, COLORREF color)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		UINT numViewports = 1;
		D3D11_VIEWPORT viewport;
		D3D11_MAPPED_SUBRESOURCE mappedData;

		// Calculate VertexBuffer
		if (!this->vertexBuffer || FAILED(this->context->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData))) return false;

		this->context->RSGetViewports(&numViewports, &viewport);
		canvas_d3d11::Vertex* v = reinterpret_cast<canvas_d3d11::Vertex*>(mappedData.pData);

		v[0] = canvas_d3d11::Vertex((2.0f * (float)x / viewport.Width - 1.0f), (1.0f - 2.0f * (float)y / viewport.Height), 0.5f, 0.0f, 0.0f, color); // Vertex 1
		v[1] = canvas_d3d11::Vertex((2.0f * (float)(x + _width) / viewport.Width - 1.0f), (1.0f - 2.0f * (float)y / viewport.Height), 0.5f, 1.0f, 0.0f, color); // Vertex 2
		v[2] = canvas_d3d11::Vertex((2.0f * (float)(x + _width) / viewport.Width - 1.0f), (1.0f - 2.0f * (float)(y + _height) / viewport.Height), 0.5f, 1.0f, 1.0f, color); // Vertex 3
		v[3] = canvas_d3d11::Vertex((2.0f * (float)x / viewport.Width - 1.0f), (1.0f - 2.0f * (float)(y + _height) / viewport.Height), 0.5f, 0.0f, 1.0f, color); // Vertex 4

		this->context->Unmap(this->vertexBuffer, 0);

		return true;
	}

	void canvas_d3d11::draw(int32_t x, int32_t y, COLORREF color)
	{
		this->draw(x, y, this->width, this->height, color);
	}

	void canvas_d3d11::draw(int32_t x, int32_t y, uint32_t _width, uint32_t _height, COLORREF color)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		if (!this->context) return;

		UINT offset = 0;
		UINT stride = sizeof(canvas_d3d11::Vertex);

		if (this->requiresUpdate)
		{
			this->performUpdate(this->buffer);
			this->requiresUpdate = false;
		}

		canvas_d3d11::Context $(this->context);

		this->translateVertices(x, y, _width, _height, color);

		const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		this->context->OMSetBlendState(this->blendState, blendFactor, 0xffffffff);
		this->context->OMSetDepthStencilState(this->depthStencilState, 1);
		this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->effectShaderResourceVariable->SetResource(this->shaderResourceView);
		this->effectTechnique->GetPassByIndex(0)->Apply(0, this->context);
		this->context->IASetIndexBuffer(this->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		this->context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
		this->context->IASetInputLayout(this->inputLayout);
		this->context->DrawIndexed(6, 0, 0);
	}

	bool canvas_d3d11::isInitialized()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return (this->device && this->context);
	}

	bool canvas_d3d11::isLoaded()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return (this->texture && this->shaderResourceView);
	}

	ID3D11Texture2D* canvas_d3d11::getTexture()
	{
		return this->texture;
	}

	COLORREF canvas_d3d11::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return RGB(r, g, b) | (a << 24);
	}
}
