#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

#define reset_resource(resource) resource = NULL
#define release_resource(resource) if(resource) { resource->Release(); reset_resource(resource); }

namespace gameoverlay
{
	class canvas_d3d9
	{
	private:
		std::recursive_mutex mutex;

		ID3DXSprite*       sprite;
		IDirect3DDevice9*  device;
		IDirect3DTexture9* texture;

		D3DFORMAT          format;

		void reset_resources();
		bool create_resources();

		bool perform_update(const void* buffer);

		void free_buffer();

	public:
		uint32_t width;
		uint32_t height;

		void* buffer;
		bool requires_update;
		uint32_t bytes_per_pixel;

		canvas_d3d9();
		canvas_d3d9(IDirect3DDevice9* device);
		~canvas_d3d9();

		IDirect3DDevice9* get_device();

		bool create(std::string file);
		bool create(uint32_t width, uint32_t height, D3DFORMAT format, const void* buffer = NULL);
		
		bool update(const void* buffer);
		bool update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback);

		bool resize(uint32_t width, uint32_t height);

		bool draw(int32_t x, int32_t y, uint32_t width, uint32_t height, COLORREF color = -1);
		bool draw(int32_t x, int32_t y, COLORREF color = -1);

		bool is_initialized();
		bool is_loaded();

		void release_resources();
		bool initialize(IDirect3DDevice9* device);
		void reinitialize(IDirect3DDevice9* device);
	};
}
