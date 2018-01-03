#include "std_include.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer()
	{
		this->hook.on_frame(std::bind(&renderer::frame_callback, this, std::placeholders::_1));
	}

	renderer::~renderer()
	{

	}

	bool renderer::is_available()
	{
		return this->presented;
	}

	HWND renderer::get_window()
	{
		return nullptr;
	}

	icanvas* renderer::get_canvas()
	{
		if(this->d3d11_canvas.is_available()) return &this->d3d11_canvas;

		return nullptr;
	}

	void renderer::register_frame_callback(std::function<void()> _callback)
	{
		this->callback = _callback;
	}

	void renderer::unregister_frame_callback()
	{
		this->callback = {};
	}

	void renderer::frame_callback(void* swap_chain)
	{
		ID3D11Device* device11;
		ID3D11DeviceContext* context11;
		IDXGISwapChain* dxgi_swap_chain = reinterpret_cast<IDXGISwapChain*>(swap_chain);
		dxgi_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&device11);
		device11->GetImmediateContext(&context11);

		UINT num = 1;
		D3D11_VIEWPORT viewport;
		context11->RSGetViewports(&num, &viewport);

		uint32_t new_width = uint32_t(viewport.Width);
		uint32_t new_height = uint32_t(viewport.Height);

		if (!d3d11_canvas.is_initialized())
		{
			d3d11_canvas.initialize(device11);
			d3d11_canvas.create(new_width, new_height, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (d3d11_canvas.is_initialized())
		{
			if (d3d11_canvas.get_width() != new_width || d3d11_canvas.get_height() != new_height)
			{
				d3d11_canvas.resize(new_width, new_height);
			}
		}

		std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(new_width * new_height * 4);
		for (unsigned int i = 0; i < new_width * new_height * 4; ++i)
		{
			buffer[i] = std::uint8_t(rand());
		}

		d3d11_canvas.update(buffer.get());

		d3d11_canvas.draw(0, 0);

		this->presented = true;
		if (this->callback) this->callback();
	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
