#pragma once

#include <d3d11.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dx11Effect.h>
#include <WICTextureLoader.h>

#define ResetResource(resource) resource = NULL
#define ReleaseResource(resource) if(resource) { resource->Release(); ResetResource(resource); }

namespace gameoverlay
{
	class canvas_d3d11
	{
		class Context
		{
		public:
			Context(ID3D11DeviceContext* _context) : context(_context)
			{
				this->context->AddRef();
				this->context->OMGetDepthStencilState(&this->depthStencilState, &this->ref);
			}

			~Context()
			{
				this->context->OMSetDepthStencilState(this->depthStencilState, this->ref);
				this->context->Release();
			}

		private:
			UINT ref;
			ID3D11DepthStencilState* depthStencilState;

			ID3D11DeviceContext* context;
		};

		struct Vertex
		{
			Vertex() {}
			Vertex(float x, float y, float z, float u, float v, COLORREF col) : pos(x, y, z), texCoord(u, v), color(col) {}

			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 texCoord;
			COLORREF color;
		};

	private:
		std::recursive_mutex mutex;

		ID3D11DeviceContext*                 context;
		ID3D11Device*                        device;
		ID3D11Texture2D*                     texture;
		ID3D11ShaderResourceView*            shaderResourceView;
		ID3D11Buffer*                        indexBuffer;
		ID3D11Buffer*                        vertexBuffer;
		ID3D10Blob*                          shaderBuffer;
		ID3D11InputLayout*                   inputLayout;
		ID3D11DepthStencilState*             depthStencilState;
		ID3D11BlendState*                    blendState;
		ID3DX11Effect*                       effect;
		ID3DX11EffectTechnique*              effectTechnique;
		ID3DX11EffectShaderResourceVariable* effectShaderResourceVariable;

		void resetResources();
		bool createResources();
		bool translateVertices(int32_t x, int32_t y, uint32_t width, uint32_t height, COLORREF color);

		void freeBuffer();

		bool performUpdate(const void* buffer);

	public:
		Utils::Memory::Allocator allocator;

		uint32_t width;
		uint32_t height;

		void* buffer;
		bool requiresUpdate;
		uint32_t bytesPerPixel;

		canvas_d3d11();
		canvas_d3d11(ID3D11Device* pDevice);
		~canvas_d3d11();

		bool create(std::string file);
		bool create(uint32_t width, uint32_t height, DXGI_FORMAT format, const void* buffer = NULL);

		bool update(const void* buffer);
		bool update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback);

		bool resize(uint32_t width, uint32_t height);

		void draw(int32_t x, int32_t y, uint32_t width, uint32_t height, COLORREF color = -1);
		void draw(int32_t x, int32_t y, COLORREF color = -1);

		void releaseResources();
		bool initialize(ID3D11Device* pDevice);
		bool isInitialized();
		bool isLoaded();

		ID3D11Texture2D* getTexture();

		static COLORREF Color(uint8_t r = -1, uint8_t g = -1, uint8_t b = -1, uint8_t a = -1);
	};
}
