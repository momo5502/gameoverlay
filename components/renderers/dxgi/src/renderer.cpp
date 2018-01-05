#include "std_include.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer() : swap_chain(nullptr)
	{
		this->hook_dxgi.on_frame(std::bind(&renderer::frame_callback, this, std::placeholders::_1));
	}

	renderer::~renderer()
	{

	}

	bool renderer::is_available()
	{
		return this->swap_chain != nullptr;
	}

	HWND renderer::get_window()
	{
		if (!this->swap_chain) return nullptr;

		IDXGISwapChain* dxgi_swap_chain = reinterpret_cast<IDXGISwapChain*>(this->swap_chain);

		DXGI_SWAP_CHAIN_DESC desc;
		dxgi_swap_chain->GetDesc(&desc);

		return desc.OutputWindow;
	}

	icanvas* renderer::get_canvas()
	{
		if (!this->dxgi_canvas.is_available()) return nullptr;
		return &this->dxgi_canvas;
	}

	void renderer::register_frame_callback(std::function<void()> _callback)
	{
		this->callback = _callback;
		if (this->is_available()) this->callback();
	}

	void renderer::unregister_frame_callback()
	{
		this->callback = {};
	}

	void renderer::frame_callback(void* _swap_chain)
	{
		this->swap_chain = _swap_chain;
		if (this->callback) this->callback();

		IDXGISwapChain* dxgi_swap_chain = reinterpret_cast<IDXGISwapChain*>(this->swap_chain);
		this->dxgi_canvas.draw(dxgi_swap_chain);
	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
