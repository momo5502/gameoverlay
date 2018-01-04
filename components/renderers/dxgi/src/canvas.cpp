#include "std_include.hpp"
#include "canvas.hpp"

namespace gameoverlay
{
	canvas::~canvas()
	{

	}

	bool canvas::is_available()
	{
		return this->is_d3d10_available() || this->is_d3d11_available();
	}

	uint32_t canvas::get_width()
	{
		if (this->is_d3d10_available()) {}
		if (this->is_d3d11_available()) return this->d3d11_canvas.width;
		return 0;
	}

	uint32_t canvas::get_height()
	{
		if (this->is_d3d10_available()) {}
		if (this->is_d3d11_available()) return this->d3d11_canvas.height;
		return 0;
	}

	bool canvas::paint(const void* buffer)
	{
		// TODO: Do channel swapping here
		if(this->is_d3d10_available()) {}
		if (this->is_d3d11_available()) return this->d3d11_canvas.update(buffer);
		return false;
	}

	void canvas::draw(IDXGISwapChain* swap_chain)
	{
		if (!swap_chain) return;

		ID3D10Device* device10 = nullptr;
		swap_chain->GetDevice(__uuidof(ID3D10Device), reinterpret_cast<void**>(&device10));
		this->draw_d3d10(device10);

		ID3D11Device* device11 = nullptr;
		swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device11));
		this->draw_d3d11(device11);
	}

	bool canvas::is_d3d10_available()
	{
		return false;
	}

	bool canvas::is_d3d11_available()
	{
		return this->d3d11_canvas.is_loaded();
	}

	void canvas::draw_d3d10(ID3D10Device* device)
	{
		if (!device) return;
	}

	void canvas::draw_d3d11(ID3D11Device* device)
	{
		if (!device) return;

		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);
		if (!context) return;

		UINT num = 1;
		D3D11_VIEWPORT viewport;
		context->RSGetViewports(&num, &viewport);

		uint32_t new_width = uint32_t(viewport.Width);
		uint32_t new_height = uint32_t(viewport.Height);

		if (!this->d3d11_canvas.is_initialized())
		{
			this->d3d11_canvas.initialize(device);
			this->d3d11_canvas.create(new_width, new_height, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (this->d3d11_canvas.is_initialized())
		{
			if (this->d3d11_canvas.width != new_width || this->d3d11_canvas.height != new_height)
			{
				d3d11_canvas.resize(new_width, new_height);
			}
		}

		this->d3d11_canvas.draw(0, 0);
	}
}
