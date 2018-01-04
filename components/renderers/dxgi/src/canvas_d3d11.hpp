#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dx11Effect.h>
#include <WICTextureLoader.h>

#define reset_resource(resource) resource = NULL
#define release_resource(resource) if(resource) { resource->Release(); reset_resource(resource); }

namespace gameoverlay
{
	class canvas_d3d11
	{
		class context_store
		{
		public:
			context_store(ID3D11DeviceContext* _context) : context(_context)
			{
				this->context->AddRef();
				this->context->OMGetDepthStencilState(&this->depth_stencil_state, &this->ref);
			}

			~context_store()
			{
				this->context->OMSetDepthStencilState(this->depth_stencil_state, this->ref);
				this->context->Release();
			}

		private:
			UINT ref;
			ID3D11DepthStencilState* depth_stencil_state;

			ID3D11DeviceContext* context;
		};

		struct vertex
		{
			vertex() {}
			vertex(float x, float y, float z, float u, float v, COLORREF col) : pos(x, y, z), tex_coord(u, v), color(col) {}

			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 tex_coord;
			COLORREF color;
		};

	private:
		std::recursive_mutex mutex;

		ID3D11DeviceContext*                 context;
		ID3D11Device*                        device;
		ID3D11Texture2D*                     texture;
		ID3D11ShaderResourceView*            shader_resource_view;
		ID3D11Buffer*                        index_buffer;
		ID3D11Buffer*                        vertex_buffer;
		ID3D10Blob*                          shader_buffer;
		ID3D11InputLayout*                   input_layout;
		ID3D11DepthStencilState*             depth_stencil_state;
		ID3D11BlendState*                    blend_state;
		ID3DX11Effect*                       effect;
		ID3DX11EffectTechnique*              effect_technique;
		ID3DX11EffectShaderResourceVariable* effect_shader_resource_variable;

		void reset_resources();
		bool create_resources();
		bool translate_vertices(int32_t x, int32_t y, uint32_t width, uint32_t height, COLORREF color);

		void free_buffer();

		bool perform_update(const void* buffer);

	public:
		uint32_t width;
		uint32_t height;

		void* buffer;
		bool requires_update;
		uint32_t bytes_per_pixel;

		canvas_d3d11();
		canvas_d3d11(ID3D11Device* device);
		~canvas_d3d11();

		ID3D11Device* get_device();

		bool create(std::string file);
		bool create(uint32_t width, uint32_t height, DXGI_FORMAT format, const void* buffer = NULL);

		bool update(const void* buffer);
		bool update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback);

		bool resize(uint32_t width, uint32_t height);

		void draw(int32_t x, int32_t y, uint32_t width, uint32_t height, COLORREF color = -1);
		void draw(int32_t x, int32_t y, COLORREF color = -1);

		void release_resources();
		bool initialize(ID3D11Device* device);
		bool is_initialized();
		bool is_loaded();

		ID3D11Texture2D* get_texture();
	};
}
