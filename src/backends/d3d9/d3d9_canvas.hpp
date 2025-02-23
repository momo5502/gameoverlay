#pragma once

#include "../../canvas.hpp"

namespace gameoverlay::d3d9
{
	class canvas : public ::fixed_canvas
	{
	public:
		canvas(IDirect3DDevice9* device, uint32_t width, uint32_t height);
		~canvas() override = default;

		void paint(const void* image) override;

		void draw() const;

	private:
		IDirect3DDevice9* device_{};
		CComPtr<ID3DXSprite> sprite_{};
		CComPtr<IDirect3DTexture9> texture_{};
	};
}
