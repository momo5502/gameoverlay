#include "std_include.hpp"

#include "hook.hpp"

namespace gameoverlay
{
	hook* hook::instance = nullptr;

	void hook::frame_handler(HDC hdc)
	{
		if (!hdc || !this->glew_initialized.has_value() || !this->glew_initialized.value()) return;
		if (this->frame_callback) this->frame_callback(hdc);
	}

	void hook::on_frame(std::function<void(HDC)> callback)
	{
		this->frame_callback = callback;
	}

	BOOL WINAPI hook::swap_buffers(HDC hdc)
	{
		if (!this->glew_initialized.has_value())
		{
			this->glew_initialized.emplace(glewInit() == GLEW_OK);
		}

		this->frame_handler(hdc);
		return this->swapBuffers_hook.invoke_pascal<bool>(hdc);
	}

	BOOL WINAPI hook::swap_buffers_stub(HDC hdc)
	{
		return hook::instance->swap_buffers(hdc);
	}

	hook::hook()
	{
		hook::instance = this;

		this->swapBuffers_hook.create(::SwapBuffers, hook::swap_buffers_stub);
	}

	hook::~hook()
	{
		hook::instance = nullptr;
	}
}
