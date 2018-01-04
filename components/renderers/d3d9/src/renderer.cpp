#include "std_include.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer()
	{

	}

	renderer::~renderer()
	{

	}

	bool renderer::is_available()
	{
		return false;
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

	}

	void renderer::unregister_frame_callback()
	{

	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
