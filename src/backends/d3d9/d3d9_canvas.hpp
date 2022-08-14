#pragma once

#include "../../canvas.hpp"

namespace gameoverlay::d3d9
{
	class canvas : public ::canvas
	{
	public:
		canvas(IDirect3DDevice9* device, uint32_t width, const uint32_t height);
		~canvas() override = default;

		dimensions get_dimensions() const override;
		void paint(const void* image) override;

		void draw() const;

	private:
		IDirect3DDevice9* device_{};
		CComPtr<ID3DXSprite> sprite_{};
		CComPtr<IDirect3DTexture9> texture_{};

		const uint32_t width_{};
		const uint32_t height_{};
	};
}
