#include "std_include.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer() : hdc(nullptr)
	{
		this->hook.on_frame(std::bind(&renderer::frame_callback, this, std::placeholders::_1));
	}

	renderer::~renderer()
	{

	}

	bool renderer::is_available()
	{
		return hdc != nullptr;
	}

	HWND renderer::get_window()
	{
		if (!this->hdc) return nullptr;
		return nullptr;
	}

	icanvas* renderer::get_canvas()
	{
		return &this->canvas;
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

	void renderer::frame_callback(HDC _hdc)
	{
		this->hdc = _hdc;
		if (this->callback) this->callback();

		this->canvas.draw(hdc);
	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
