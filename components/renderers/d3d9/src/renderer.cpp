#include "std_include.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
	renderer::renderer() : device(nullptr)
	{
		this->d3d9_hook.on_frame(std::bind(&renderer::frame_callback, this, std::placeholders::_1));
		this->d3d9_hook.on_reset(std::bind(&renderer::reset_callback, this, std::placeholders::_1));

		this->d3d9ex_hook.on_frame(std::bind(&renderer::frame_callback, this, std::placeholders::_1));
		this->d3d9ex_hook.on_reset(std::bind(&renderer::reset_callback, this, std::placeholders::_1));
	}

	renderer::~renderer()
	{

	}

	bool renderer::is_available()
	{
		return device != nullptr;
	}

	HWND renderer::get_window()
	{
		if (!this->device) return nullptr;

		IDirect3DDevice9* d3d9_device = reinterpret_cast<IDirect3DDevice9*>(this->device);

		D3DDEVICE_CREATION_PARAMETERS params;
		d3d9_device->GetCreationParameters(&params);

		return params.hFocusWindow;
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

	void renderer::frame_callback(void* _device)
	{
		this->device = _device;
		if (this->callback) this->callback();

		IDirect3DDevice9* d3d9_device = reinterpret_cast<IDirect3DDevice9*>(this->device);
		this->canvas.draw(d3d9_device);
	}

	void renderer::reset_callback(void* /*_device*/)
	{
		this->canvas.reset();
	}

	extern "C" __declspec(dllexport) renderer* create_interface()
	{
		static renderer _renderer;
		return &_renderer;
	}
}
