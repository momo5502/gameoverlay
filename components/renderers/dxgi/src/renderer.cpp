#include "std_include.hpp"

#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer() : hook(this)
	{
		printf("Initializing DXGI renderer...\n");
	}

	renderer::~renderer()
	{
		printf("Uninitializing DXGI renderer...\n");
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

	void renderer::frame_callback()
	{
		MessageBoxA(0, "WOHOO", 0, 0);

		this->presented = true;
		if (this->callback) this->callback();
	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
